#include "station.h"
#include "parser.h"
#include <iostream>
#include <unistd.h>
#include <algorithm>

using namespace std;

Station::Station(bool routerFlag, string ifaceFile, string rtableFile, string hostFile)
{
	m_router = routerFlag;
	
	m_ifaces = extractInterfaces(ifaceFile);
	m_rTableEntries = extractRouteTable(rtableFile);
	m_hostMap = extractHosts(hostFile);
	
	// No need to initialize m_fd vector.  We will push back values as needed
}

SocketBufferEntry Station::createSbEntry(IPAddr ip, vector<unsigned char> bytes)
{
	SocketBufferEntry sbEntry;
	
	// Try to find a match between the ip and each mask in the routing table so we know
	// where to send this packet
	bool found = false;
	for(unsigned int i = 0; i < m_rTableEntries.size(); ++i) {
		// If the masked ip matches a destination subnet, then we know where to send the packet next
		if((ip & m_rTableEntries[i].mask) == m_rTableEntries[i].destsubnet) {
			string iface(m_rTableEntries[i].ifacename);

			auto it = m_fdLookup.find(iface);
			if(it != m_fdLookup.end()) {
				// We found it!
				found = true;
				sbEntry.fd = it->second;
				sbEntry.bytes.assign(bytes.begin(), bytes.end());
				break;
			}
		}	
	}
	
	// TODO: return sbEntry with some fd that lets us know there is no route?
	if(!found) {
		cout << "fdLookup failed.  Exiting." << endl;
		exit(1);
	}
	
	return sbEntry;
}

/**
 * When a packet has arrived, we need to determine if it is an ARP or IP.
 * Depending on what kind of packet it is, we need to either:
 * 	display the message (in case we've received an IP packet meant for us)
 * 	generate an ARP reply (in case we've received an ARP request that we can satisfy)
 * 	discard the packet (in case the packet is not for us or we cannot satisfy the ARP request)
 */
void Station::handlePacket(char inputBuffer[BUFSIZE], int incomingFd)
{
	// The packet will come to us as an EtherPkt.  Determine if the EtherPkt is wrapping
	// an IP packet or ARP pkt
	EtherPkt etherPkt = writeBytesToEtherPacket(inputBuffer);
	
	// If we have received an ARP Packet, we need to know if it is a request or a reply
	if(etherPkt.type == TYPE_ARP_PKT) {
		ARP_PKT arpPkt = writeBytesToArpPkt(etherPkt.data);
		
		// If we have received a packet destined for us that we sent, drop it
		if(!router() && arpPkt.dstip != ip()) {
			return;
		}

		// We need to find the next hop and forward the packet
		if(router()) {
			for(unsigned int i = 0; i < m_rTableEntries.size(); ++i) {
				if(m_rTableEntries[i].destsubnet == (m_rTableEntries[i].mask & arpPkt.dstip)) {
					// Found it!  Get the next hop and send the packet that way
					SocketBufferEntry pktToSend = createSbEntry(arpPkt.dstip, writeEthernetPacketToBytes(etherPkt));
					if(pktToSend.fd != incomingFd)
						m_pendingQueue.push_back(pktToSend);
					break;
				}
			}
			return;
		}
		
		insertArpCache(arpPkt.srcip, arpPkt.srcmac);
		
		if(arpPkt.op == ARP_REQUEST) {
			constructArpReply(arpPkt);
		}
		// We've received a reply, which means that we can map the IP and MACs
		else if(arpPkt.op == ARP_REPLY) {
			moveFromArpWaitToPQ(arpPkt);
		}
		else {
			cout << "ARP OP UNKNOWN.  Bad error, exiting before things break even more." << endl;
			exit(1);
		}	
	}
	else if(etherPkt.type == TYPE_IP_PKT) {
		IP_PKT ipPkt = writeBytesToIpPkt(etherPkt.data);
		// If we have received a packet destined for us that we sent, drop it
		if(!router() && ipPkt.dstip != ip()) {
			return;
		}
		// If we are not a router, display the data buffer.  
		// If we are a router, forward the packet
		if(!router())
			cout << ipPkt.data << endl;
		else {
			// We need to find the next hop and forward the packet
			for(unsigned int i = 0; i < m_rTableEntries.size(); ++i) {
				if(m_rTableEntries[i].destsubnet == (m_rTableEntries[i].mask & ipPkt.dstip)) {
					// Found it!  Get the next hop and send the packet that way
					SocketBufferEntry pktToSend = createSbEntry(ipPkt.dstip, writeEthernetPacketToBytes(etherPkt));
					if(pktToSend.fd != incomingFd)
						m_pendingQueue.push_back(pktToSend);
					break;
				}
			}
			
		}
	}

}

/**
 * Parse through commands typed by the user and perform the appropriate I/O
 */
void Station::handleUserInput(char inputBuffer[BUFSIZE])
{
	string line(inputBuffer);
	
	stringstream linestream(line);
	
	// only handle lowercase commands
	string command;
	linestream >> command;
	transform(command.begin(), command.end(), command.begin(), ::tolower);
	
	if(command == "send") {
		string dstHost;
		linestream >> dstHost;
		
		string data;
		getline(linestream, data);
		
		// First, construct an IP packet.  This will be encapsulated in an EtherPkt,
		// which will need to handle finding the destination MAC address
		IP_PKT ipPkt;
		ipPkt.srcip = ip();

		// Try to find the IP in our host file.
		// TODO:  what if the host isn't in our host file?
		ipPkt.dstip = m_hostMap.find(dstHost)->second;
		
		strcpy(ipPkt.data, data.c_str());
		
		ipPkt.length = sizeof(ipPkt.data);
		
		// Now, construct the ethernet packet and lookup the destination mac address
		EtherPkt etherPkt;
		
		strcpy(etherPkt.src, mac().c_str());
		
		etherPkt.type = TYPE_IP_PKT;
		vector<unsigned char> ipBytes = writeIpPktToBytes(ipPkt);
		
		memcpy(etherPkt.data, &ipBytes[0], ipBytes.size());
		
		etherPkt.size = ipBytes.size();
		
		// Lookup mac address in ARP cache for destination.  If we can't find it, then store the packet in a queue
		// of packets waiting on ARP replies and send out an ARP request.
		auto cacheItr = m_arpCache.find(ipPkt.dstip);
		if(cacheItr == m_arpCache.end()) {
			cout << "Could not find " << ntop(ipPkt.dstip) << " in the cache.  Sending ARP request" << endl;
			// It wasn't found, send ARP request and add the packet to a wait queue
			constructArpRequest(ipPkt.dstip);

			m_arpWaitQueue.push_back(etherPkt);
		}
		else {
			cout << "Found " << ntop(ipPkt.dstip) << " in the cache!" << endl;
			// It was found, drop the packet in the pending queue.  Update the timestamp for the cache entry,
			// as it is being accessed
			gettimeofday(&(cacheItr->second.timeStamp), NULL);
			strcpy(etherPkt.dst, cacheItr->second.mac);

			SocketBufferEntry sbEntry = createSbEntry(ipPkt.dstip, writeEthernetPacketToBytes(etherPkt));
			
			m_pendingQueue.push_back(sbEntry);
		}
	}

	else if(command == "show")
	{
		string target;
		linestream >> target;

		if(target == "arp") {
			displayArpCache();
			dumpFdLookup();
		}
		else if(target == "pq")
			displayPQ();
		else if(target == "host")
			displayHostMap();
		else if(target == "iface")
			displayInterfaces();
		else if(target == "rtable")
			displayRouteTable();
		
		else //Invalid show command
		{
			cout << "Invalid show command" << endl;
			cout << " Valid options: arp, pq, host, iface, rtable" << endl << endl;
		}
	}
	else if(command == "quit")
		exit(0);
	else {
		cout << "Invalid command: " << inputBuffer << endl;
		cout << "Valid commands: " << endl;
		cout << "send <host> <message>" << endl;
		cout << "show <arp|pq|host|iface|rtable" << endl;
	}
}

vector< int > Station::sockets()
{
	return m_fd;
}

bool Station::router()
{
	return m_router;
}

bool Station::isSocket(int fd)
{
	for(unsigned int i = 0; i < m_fd.size(); ++i) {
		if(m_fd[i] == fd)
			return true;
	}
		
	return false;
}


/**
 * Iterate through the pending packet queue and send everything out
 */
void Station::sendPendingPackets()
{
	if(m_pendingQueue.size() <= 0) {
		return;
	}
	
	for(unsigned int i = 0; i < m_pendingQueue.size(); ++i) {
		char buf[BUFSIZE];
		for(unsigned int i = 0; i < sizeof(buf); ++i)
				buf[i] = 0;
		
		memcpy(&buf, &m_pendingQueue[i].bytes[0], m_pendingQueue[i].bytes.size());
		
		if(send(m_pendingQueue[i].fd, buf, sizeof(buf), 0) <= -1) {
			cout << "Could not send PQ[" << i << "]: " << &m_pendingQueue[i].bytes[0] << endl;
			cout << "buf: " << buf << endl;
		}
		else
			cout << "Successfully sent a packet!" << endl;
	}

	m_pendingQueue.clear();
}


/**
 * Construct the request for the MAC address of our EtherPkt's destination
 * and add it to the queue of packets waiting to be sent out.  Remember that
 * packets are sent out on the Mac layer, therefore we need to add the bytes of an
 * EtherPkt to our pending packet queue (NOT THE BYTES OF AN ARP PKT).  The arp pkt is wrapped
 * inside of an EtherPkt
 */
void Station::constructArpRequest(IPAddr dstip)
{
	EtherPkt etherPkt;
	
	strcpy(etherPkt.src, mac().c_str());
	etherPkt.type = TYPE_ARP_PKT;
	
	// We know everything except for the destination MAC address
	ARP_PKT arpPkt;
	
	arpPkt.op = 0;
	arpPkt.srcip = ip();
	arpPkt.dstip = dstip;
	strcpy(arpPkt.srcmac, mac().c_str());
	
	strcpy(arpPkt.dstmac, etherPkt.dst);

	// The ARP packet will be contained in the EtherPkt's data buffer
	vector<unsigned char> arpBytes = writeArpPktToBytes(arpPkt);
	
	etherPkt.size = arpBytes.size();
	memcpy(&etherPkt.data, &arpBytes[0], arpBytes.size());
	
	// Need EtherPkt bytes for pendingQueue

	SocketBufferEntry sbEntry = createSbEntry(dstip, writeEthernetPacketToBytes(etherPkt));
	m_pendingQueue.push_back(sbEntry);
}

void Station::constructArpReply(ARP_PKT general)
{
	ARP_PKT arpPkt;

	arpPkt.op = 1;

	arpPkt.srcip = general.dstip;

	strcpy(arpPkt.srcmac, mac().c_str());
	
	arpPkt.dstip = general.srcip;

	strcpy(arpPkt.dstmac, general.srcmac);
	
	EtherPkt ePkt;

	strcpy(ePkt.dst, arpPkt.dstmac);
	strcpy(ePkt.src, arpPkt.srcmac);
	
	ePkt.type = 0;

	vector<unsigned char> arpBytes = writeArpPktToBytes(arpPkt);

	ePkt.size = arpBytes.size();
	memcpy(&ePkt.data, &arpBytes[0], arpBytes.size());

	SocketBufferEntry sbEntry = createSbEntry(arpPkt.dstip, writeEthernetPacketToBytes(ePkt));
	m_pendingQueue.push_back(sbEntry);
}

/**
 * Get our IP address.  TODO:  support multiple NICs
 */
IPAddr Station::ip()
{
	return m_ifaces[0].ipaddr;
}

/**
 * Get our MAC address.  TODO:  support multiple NICs
 */
string Station::mac()
{
	return m_ifaces[0].macaddr;
}



void Station::displayArpCache()
{
	cout << "ARP CACHE" << endl;
	cout << "IP ADDRESS\tMAC ADDRESS\tTIMESTAMP" << endl;
	for(auto &it : m_arpCache) {
		cout << ntop(it.first) << "\t";
		cout << it.second.mac << "\t";
		cout << it.second.timeStamp.tv_sec;
		cout << endl;
	}
}

void Station::displayPQ()
{
	cout << "PENDING QUEUE CONTENTS" << endl;
	for(unsigned int i = 0; i < m_pendingQueue.size(); i++)
	{
		for(unsigned int j = 0; j < m_pendingQueue[i].bytes.size(); j++)
		{
			cout << m_pendingQueue[i].bytes[j];	
		}
	}
	cout << endl;
}


void Station::displayInterfaces()
{
	cout << "INTERFACES" << endl;
	cout << "NAME\tIP\tSUBNET\tMAC\tLAN" << endl;
	for(unsigned int i = 0; i < m_ifaces.size(); ++i) {
		cout << m_ifaces[i].ifacename << "\t";
		cout << ntop(m_ifaces[i].ipaddr) << "\t";
		cout << ntop(m_ifaces[i].mask) << "\t";
		
		cout << m_ifaces[i].macaddr << "\t";

		cout << m_ifaces[i].lanname;
		cout << endl;
	}
	cout << endl;
}

void Station::displayRouteTable()
{
	cout << "ROUTING TABLE" << endl;
	cout << "DESTINATION\tNEXT HOP\tMASK\tINTERFACE" << endl;
	for (unsigned int i = 0; i < m_rTableEntries.size(); ++i) {
		cout << ntop(m_rTableEntries[i].destsubnet) << "\t";
		cout << ntop(m_rTableEntries[i].nexthop) << "\t";
		cout << ntop(m_rTableEntries[i].mask) << "\t";
		cout << m_rTableEntries[i].ifacename;
		cout << endl;
	}
	cout << endl;
}

void Station::displayHostMap()
{
	cout << "HOST INFORMATION" << endl;
	cout << "HOSTNAME\tIP ADDRESS" << endl;
	for(auto &it : m_hostMap) {
		cout << it.first << "\t";
		cout << ntop(it.second) << "\t";
		cout << endl;
	}
}

void Station::dumpFdLookup()
{
	cout << "FD LOOKUP" << endl;
	cout << "IFACENAME\tFD" << endl;
	for(auto &it : m_fdLookup) {
		cout << it.first << "\t";
		cout << it.second << "\t";
		cout << endl;
	}
}


/**
 * Attempt to connect to a bridge by looking at the .info file created
 * by a bridge.
 */
void Station::connectToBridge()
{
	if(m_ifaces.size() <= 0)
		return;
	
	for(unsigned int i = 0; i < m_ifaces.size(); ++i) {
		string bridgeName(m_ifaces[i].lanname);
		
		string bridgeFile = bridgeName + ".info";
		
		// Tried to open a newline from iface file?
		if(bridgeFile == ".info")
			continue;
		
		ifstream infile(bridgeFile.c_str());
		
		if(!infile.is_open()) {
			cout << "Error opening " << bridgeFile << " ...skipping this bridge." << endl;
			return;
		}
		
		string line;
		
		getline(infile, line);
		stringstream linestream(line);
		
		string addr;
		string port;
		
		linestream >> addr >> port;
		
		struct addrinfo *res;
		
		struct addrinfo hints;
		memset(&hints, 0, sizeof hints);

		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;
		
		getaddrinfo(addr.c_str(), port.c_str(), &hints, &res);
		
		int fd = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);

		bool connected = false;
		for(int j = 0; j < 5; ++j) {
			if(connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
				perror("connect()");
				return;
			}
			
			cout << "Connected to " << addr << endl;
			
			// We must listen for an "accept" or "reject" string from the bridge
			int bytesRead = 0;
			char buf[BUFSIZE];
			if((bytesRead = recv(fd, buf, sizeof buf, 0)) <= 0) {
				cout << "recv error while listening for accept/reject" << endl;
				return;
			}

			if(strcmp(buf, "Reject") == 0) {
				cout << addr << " rejected our connection!" << endl;
				close(fd);
				return;
			}
			else if (strcmp(buf, "Accept") == 0) {
				cout << addr << " accepted our connection!" << endl;
				m_fd.push_back(fd);
				//cout << __func__ << __LINE__ << "ip addr: " << ((sockaddr_in*)res->ai_addr)->sin_addr.s_addr << endl;
				m_fdLookup.insert(pair<string, int>(m_ifaces[i].ifacename, fd));

				connected = true;
				break;
				
			}
			else  {
				cout << "Could not connect on attempt " << j << endl;
				sleep(2);
			}
			
		}
		if(!connected)
			close(fd);
	}

}

void Station::insertArpCache(IPAddr ip, MacAddr mac)
{
	CacheEntry entry;
	gettimeofday(&(entry.timeStamp), NULL);
	strcpy(entry.mac, mac);
	
	cout << "Adding " << mac << " to ARP cache." << endl;
	
	m_arpCache.insert(pair<IPAddr, CacheEntry>(ip, entry));
}

CacheEntry Station::lookupArpCache(IPAddr ip)
{
	auto it = m_arpCache.find(ip);
	
	//if this is true then ip address is not in the table
	if(it == m_arpCache.end()) {
		CacheEntry empty;
		strcpy(empty.mac, "");
		empty.timeStamp.tv_sec = 0;
		return empty;
	}

	gettimeofday(&(it->second.timeStamp), NULL);
	return it->second;
}

/**
 * Move a packet that has received an ARP reply from the wait queue to the pending queue,
 * because now we know where to send the waiting packet to
 */
void Station::moveFromArpWaitToPQ(ARP_PKT arpPkt)
{
	for(unsigned int i = 0; i < m_arpWaitQueue.size(); i++) {
		if(strcmp(m_arpWaitQueue[i].src, arpPkt.dstmac) == 0) {
			EtherPkt movingPkt = m_arpWaitQueue[i];
			strcpy(movingPkt.dst, arpPkt.srcmac);
			SocketBufferEntry sbEntry = createSbEntry(arpPkt.srcip, writeEthernetPacketToBytes(movingPkt));

			m_pendingQueue.push_back(sbEntry);
			m_arpWaitQueue.erase(m_arpWaitQueue.begin()+i);
		}
	}
}

void Station::arpCacheTimeout()
{
	timeval currentTime;
	gettimeofday(&currentTime, NULL);
	
	for(auto &it : m_arpCache) {
			if((currentTime.tv_sec - it.second.timeStamp.tv_sec) > 30) {
				cout << "Removing " << it.second.mac << " from ARP cache." << endl;
				m_arpCache.erase(it.first);
			}
		}
}