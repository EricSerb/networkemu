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
	
	// At construction, we have not connected to anything.  Let's make sure
	// we set a value that will not conflict with other FDs.
	m_fd = -1;
}

/**
 * When a packet has arrived, we need to determine if it is an ARP or IP.
 * Depending on what kind of packet it is, we need to either:
 * 	display the message (in case we've received an IP packet meant for us)
 * 	generate an ARP reply (in case we've received an ARP request that we can satisfy)
 * 	discard the packet (in case the packet is not for us or we cannot satisfy the ARP request)
 * 
 * TODO: if we're a router, consult the routing table to figure out where the packet should go,
 * 	then set it up to be forwarded properly and add it to the pending queue
 */
void Station::handlePacket(char inputBuffer[BUFSIZE])
{
	cout << "handlePacket input buffer: " << inputBuffer << endl;
	// The packet will come to us as an EtherPkt.  Determine if the EtherPkt is wrapping
	// an IP packet or ARP pkt
	EtherPkt etherPkt = writeBytesToEtherPacket(inputBuffer);
	cout << __LINE__ << endl;
	etherPkt.dump();
	cout << __LINE__ << endl;
	
	// If we have received an ARP Packet, we need to know if it is a request or a reply
	if(etherPkt.type == TYPE_ARP_PKT) {
		ARP_PKT arpPkt = writeBytesToArpPkt(etherPkt.data);
		arpPkt.dump();
		if(arpPkt.op == ARP_REQUEST)
			constructArpReply(arpPkt);
		// We've received a reply, which means that we can map the dest IP to a dest MAC
		else if(arpPkt.op == ARP_REPLY) {
			insertArpCache(arpPkt.dstip, arpPkt.dstmac);
			// TODO: how will we handle moving the packets that did not have a dest MAC to
			// the pending queue?
		}
		else {
			cout << "ARP OP UNKNOWN.  Bad error, exiting before things break even more." << endl;
			exit(1);
		}	
	}
	else if(etherPkt.type == TYPE_IP_PKT) {
		IP_PKT pkt = writeBytesToIpPkt(etherPkt.data);
		// If we are not a router, display the data buffer.  
		// If we are a router, forward the packet
		if(!router())
			cout << pkt.data << endl;
		else {
			// TODO: consult routing table and forward
		}
	}
	
	cout << __LINE__ << endl;
}

/**
 * Parse through commands typed by the user and perform the appropriate I/O
 */
void Station::handleUserInput(char inputBuffer[BUFSIZE])
{
	cout << "user input buffer: " << inputBuffer << endl;
	string line(inputBuffer);
	
	stringstream linestream(line);
	
	// only handle lowercase commands
	string command;
	linestream >> command;
	transform(command.begin(), command.end(), command.begin(), ::tolower);
	
	cout << "line: " << line << endl << "command: " << command << endl;
	
	if(command == "send")
	{
		//parse pkt into a buf
		string cmd, dstHost, data, newBuf(inputBuffer);

		std::size_t i = newBuf.find(" ", 0);
		
		//found lenght of command and then copy it out. should be 4 characters
		cmd = newBuf.substr(0, i); 
		i++;

		cout << "cmd: " << cmd << endl;
		//Now from position of i+1 should be start of host name and find end position of that
		std::size_t k = newBuf.find(" ", i);

		//copy out host name. Do k-i because that is the length we need to copy
		dstHost = newBuf.substr(i, (k - i));
		k++; //move k to start of data
		
		//get data out and now have everything extracted
		data = newBuf.substr(k, (newBuf.length() - k));
		
		cout << "dstHost: " << dstHost << endl << "data: " << data << endl;
		
		// First, construct an IP packet.  This will be encapsulated in an EtherPkt,
		// which will need to handle finding the destination MAC address
		IP_PKT ipPkt;
		ipPkt.srcip = ip();

		// Try to find the IP in our host file.
		// TODO:  what if the host isn't in our host file?
		ipPkt.dstip = m_hostMap.find(dstHost)->second;
		
		strcpy(ipPkt.data, data.c_str());
		
		ipPkt.length = sizeof(ipPkt.data);
		
		//ipPkt.dump();
		
		// Now, construct the ethernet packet and lookup the destination mac address
		EtherPkt etherPkt;
		
		strcpy(etherPkt.src, mac().c_str());
		
		etherPkt.type = TYPE_IP_PKT;
		vector<unsigned char> ipBytes = writeIpPktToBytes(ipPkt);
		
		memcpy(etherPkt.data, &ipBytes[0], ipBytes.size());
		
		etherPkt.size = ipBytes.size();
		
		//etherPkt.dump();
		
		// Lookup mac address in ARP cache for destination.  If we can't find it, then store the packet in a queue
		// of packets waiting on ARP replies and send out an ARP request.
		auto cacheItr = m_arpCache.find(ipPkt.dstip);
		if(cacheItr == m_arpCache.end()) {
			cout << "Could not find " << ntop(ipPkt.dstip) << " in the cache" << endl;
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
			cout << "eth.dst" << etherPkt.dst << endl;
			vector<unsigned char> ethBytes = writeEthernetPacketToBytes(etherPkt);
			m_pendingQueue.push_back(ethBytes);
		}
	}

	else if(command == "show")
	{
		string target;
		linestream >> target;
		
		cout << "target: " << target << endl;

		if(target == "arp")
			displayArpCache();
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
	else //catch for invalid commands
	{
		cout << "Invalid command: " << inputBuffer << endl;
	}
	//cout << "Skipped every option include invalid... Major error" << endl;
}

/**
 * Iterate through the pending packet queue and send everything out
 */
void Station::sendPendingPackets()
{
	if(m_pendingQueue.size() > 0)
		displayPQ();
	
	for(unsigned int i = 0; i < m_pendingQueue.size(); ++i) {
		char buf[BUFSIZE];
		for(unsigned int i = 0; i < sizeof(buf); ++i)
				buf[i] = 0;
		
		memcpy(&buf, &m_pendingQueue[i][0], m_pendingQueue[i].size());
cout << "ATTEMPTING A SEND ON LINE " << __LINE__ << " WITH BUFFER: " << buf << endl;
		if(send(socket(), buf, sizeof(buf), 0) == -1) {
			cout << "Could not send m_pendingQueue[" << i << "]: " << &m_pendingQueue[i] << endl;
			cout << "buf: " << buf << endl;
		}
		else
			cout << "sent a packet successfully! buffer is << " << buf  << endl;
	}
	
	m_pendingQueue.clear();
}


/**
 * Construct the request for the MAC address of our EtherPkt's destination
 * and add it to the queue of packets waiting to be sent out.  Remember that
 * packets are sent out on the Mac layer, therefore we need to add the bytes of an
 * EtherPkt to our pending packet queue (NOT THE BYTES OF AN ARP PKT).  The arp pkt is wrapped
 * inside of an EtherPkt
 * 
 * TODO: support multiple NICs for m_ifaces ipaddr/macaddr
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
	cout << "mac(): " << mac() << endl;
	
	strcpy(arpPkt.dstmac, etherPkt.dst);
	arpPkt.dump();

	// The ARP packet will be contained in the EtherPkt's data buffer
	vector<unsigned char> arpBytes = writeArpPktToBytes(arpPkt);
	
	etherPkt.size = arpBytes.size();
	memcpy(&etherPkt.data, &arpBytes[0], arpBytes.size());
	
	// Need EtherPkt bytes for pendingQueue
	vector<unsigned char> ethBytes = writeEthernetPacketToBytes(etherPkt);
	//etherPkt.dump()
	m_pendingQueue.push_back(ethBytes);
}

void Station::constructArpReply(ARP_PKT general)
{

	ARP_PKT pkt;

	pkt.op = 1;

	pkt.srcip = general.dstip;

	strcpy(pkt.srcmac, mac().c_str());
	
	pkt.dstip = general.srcip;

	strcpy(pkt.dstmac, general.srcmac);

	EtherPkt ePkt;

	strcpy(ePkt.dst, pkt.dstmac);
	strcpy(ePkt.src, pkt.srcmac);
	
	ePkt.type = 0;

	vector<unsigned char> arpPkt = writeArpPktToBytes(pkt);

	ePkt.size = arpPkt.size();
	memcpy(&(ePkt.data), &(arpPkt), ePkt.size);

	vector<unsigned char> ethBytes = writeEthernetPacketToBytes(ePkt);
	m_pendingQueue.push_back(ethBytes);
}


bool Station::router()
{
	return m_router;
}

int Station::socket()
{
	return m_fd;
}

/*
 * Determine whether or not a socket is closed.  Intended to be used for checks
 * against m_fd, where socket() is intended for assignments.
 */
bool Station::closed()
{
	return socket() == -1;
}


void Station::close()
{
	if(!closed())
		::close(m_fd);
	m_fd = -1;
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
		for(unsigned int j = 0; j < m_pendingQueue[i].size(); j++)
		{
			cout << m_pendingQueue[i][j];	
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

/**
 * Attempt to connect to a bridge by looking at the .info file created
 * by a bridge.
 */
void Station::connectToBridge()
{
	if(m_ifaces.size() <= 0)
		return;
	
	//TODO:  support more than just one connection (try one per interface)
	string bridgeName(m_ifaces[0].lanname);
	
	string bridgeFile = bridgeName + ".info";
	
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
	
	m_fd = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	if(connect(m_fd, res->ai_addr, res->ai_addrlen) < 0) {
		perror("connect()");
		return;
	}
	
	cout << "Connected to " << addr << endl;
	
	// We must listen for an "accept" or "reject" string from the bridge
	int bytesRead = 0;
	char buf[BUFSIZE];
	if((bytesRead = recv(m_fd, buf, sizeof buf, 0)) <= 0) {
		cout << "recv error while listening for accept/reject" << endl;
		return;
	}

	if(strcmp(buf, "Reject") == 0) {
		cout << addr << " rejected our connection!" << endl;
		close();
		return;
	}
	else if (strcmp(buf, "Accept") == 0) {
		cout << addr << " accepted our connection!" << endl;
	}
	//TODO:  try to connect 5 times, then give up (as per writeup)
	else  {
		cout << "Not sure what " << addr << " had to say about connecting to us.  Skipping it." << endl;
		close();
		return;
	}

}

void Station::insertArpCache(IPAddr ip, MacAddr mac)
{
	CacheEntry entry;
	gettimeofday(&(entry.timeStamp), NULL);
	strcpy(entry.mac, mac);

	m_arpCache.insert(pair<IPAddr, CacheEntry>(ip, entry)); 
}

CacheEntry Station::lookupArpCache(IPAddr ip)
{
	auto it = m_arpCache.find(ip);
	
	//if this is true then ip address is not in the table
	if(it == m_arpCache.end())
	{
		CacheEntry empty;
		strcpy(empty.mac, "");
		empty.timeStamp.tv_sec = 0;
		return empty;
	}

	gettimeofday(&(it->second.timeStamp), NULL);
	return it->second;
}
