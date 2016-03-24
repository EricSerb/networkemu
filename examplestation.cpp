/*-------------------------------------------------------*/

#include "parser.h"
#include <unistd.h>
#include "maclayer.h"
#include "userinterface.h"

using namespace std;
/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/* station : gets hooked to all the lans in its ifaces file, sends/recvs pkts */
/* usage: station <-no -route> interface routingtable hostname */
int main (int argc, char *argv[])
{
	/* initialization of hosts, interface, and routing tables */
	if(argc != 5) {
		cout << "Usage: station <router flag> <interface file> <routing table> <hostname>" << endl;
		exit(1);
	}
	
	// Check if we're a station or a router
	bool router;
	
	if(strcmp(argv[1], "-router") < 0)
		router = true;
	else if(strcmp(argv[1], "-no") < 0)
		router = false;
	else {
		cout << "Router flag must be `-no` or `-station`" << endl;
		exit(1);
	}
	string fn(argv[2]);
	vector<iface> ifaces = extractInterfaces(fn);
	
	dumpInterfaces(ifaces);
	
	fn = argv[3];
	
	vector<rtable> rtableEntries = extractRouteTable(fn);
	
	dumpRtables(rtableEntries);
	
	fn = argv[4];
	
	// This is essentially our DNS lookup table.
	// maps hostname to IP address
	map<string, IPAddr> hostMap = extractHosts(fn);
	
	dumpHosts(hostMap);
	
	// ARP cache, maps IP address to MAC address
	map<IPAddr, MacAddr> arpCache;

	/* hook to the lans that the station should connected to
	* note that a station may need to be connected to multilple lans
	*/
	
	fd_set masterSet;
	FD_ZERO(&masterSet);
	FD_SET(fileno(stdin), &masterSet);
	
	// The bridge sockets.  Currently hardcoded to only allow up to 2 bridges
	int sockFd[2];
	int maxFd = 0;
	
	// We need to connect to each LAN as specified in the interfaces file.
	// That means that, for each interface, we need to lookup <lan-name>.info
	// and try to connect to the address/port contained within
	for(unsigned int i = 0; i < ifaces.size(); ++i) {
		string bridgeName(ifaces[i].lanname);
		
		string bridgeFile = bridgeName + ".info";
		
		ifstream infile(bridgeFile.c_str());
		
		if(!infile.is_open()) {
			cout << "Error opening " << bridgeFile << " ...skipping this bridge." << endl;
			continue;
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
		
		sockFd[i] = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

		if(connect(sockFd[i], res->ai_addr, res->ai_addrlen) < 0) {
			perror("connect()");
			continue;
		}
		
		cout << "Connected to " << addr << endl;
		
		// We must listen for an "accept" or "reject" string from the bridge
		int bytesRead = 0;
		char buf[BUFSIZE];
		if((bytesRead = recv(sockFd[i], buf, sizeof buf, 0)) <= 0) {
			cout << "recv error while listening for accept/reject" << endl;
			return 1;
		}
	
		if(strcmp(buf, "Reject") == 0) {
			cout << addr << " rejected our connection!" << endl;
			close(sockFd[i]);
			continue;
		}
		else if (strcmp(buf, "Accept") == 0) {
			cout << addr << " accepted our connection!" << endl;
		}
		//TODO:  try to connect 5 times, then give up (as per writeup)
		else  {
			cout << "Not sure what " << addr << " had to say about connecting to us.  Skipping it." << endl;
			close(sockFd[i]);
			continue;
		}
		
		FD_SET(sockFd[i], &masterSet);
		
		if(sockFd[i] > maxFd)
			maxFd = sockFd[i];
	}
	
	while(true) {
		fd_set readSet = masterSet;
		select(maxFd+1, &readSet, NULL, NULL, NULL);
		
		int bytesRead = 0;
		char buf[BUFSIZE/2];
		memset(buf, '\0', sizeof buf);
		
		for(int i = 0; i <= maxFd; ++i) {
			if (FD_ISSET(i, &readSet)) {
				if(i == sockFd[0]) {
					// If no bytes are read, something is wrong.  Exit.
					if((bytesRead = recv(i, buf, sizeof buf, 0)) <= 0) {
						cout << "recv error in select() loop" << endl;
						return 1;
					}
					buf[bytesRead + 1] = '\0';
					cout << ">>> " << buf;
				}
				
				else if(i == fileno(stdin)) {
					bytesRead = read(i, buf, sizeof buf);
					
					if(bytesRead > 0) {
						// Need to make sure to terminate this for proper handling
						buf[bytesRead + 1] = '\0';
						
						// Parse the user commands
						if(strncmp(buf, "Send", 4) == 0 || strncmp(buf, "send", 4) == 0) {
							//parse pkt into a buf
							string cmd, dstHost, data, newBuf(buf);

							std::size_t i = newBuf.find(" ", 0);
							
							//found lenght of command and then copy it out. should be 4 characters
							cmd = newBuf.substr(0, i); 
							i++;

							//Now from position of i+1 should be start of host name and find end position of that
							std::size_t k = newBuf.find(" ", i);

							//copy out host name. Do k-i because that is the length we need to copy
							dstHost = newBuf.substr(i, (k - i));
							k++; //move k to start of data
							
							//get data out and now have everything extracted
							data = newBuf.substr(k, (newBuf.length() - k));

							//look up host from hosts map to get ip and then using the routing table get the MAC
							//if there is not MAC must send out an ARP and wait to send this message.
							auto hostMapItr = hostMap.find(dstHost);
							
							// If the host is in the hosts table, we can search for its MAC addr
							if(hostMapItr == hostMap.end()) {
								cout << "Host " << dstHost << " is not in hosts table!" << endl;
								continue;
							}
							
							//TODO:  This should first check the routing table for the next hop.  The IPAddr found in
							// the hostname file is the ENDING IP address.  Therefore, this assignment is plain wrong
							IPAddr dstIP = hostMapItr->second;
							cout << "dstIP is : " << ntop(dstIP) << endl;
							
							// Search ARP cache for the host macAddr
							auto arpCacheItr = arpCache.find(dstIP);
							
							// If the IP is not in the map, we need to queue the packet and send out an ARP
							// request.
							if(arpCacheItr == arpCache.end()) {
								//TODO: queue the packet and send an ARP request
								cout << "IP: " << ntop(dstIP) << " is not in the ARP cache!" << endl;
								continue;
							}
							
							MacAddr dstMac;
							strcpy(dstMac, arpCacheItr->second);
							
							// Create the packet and send it out
							EtherPkt pkt;
							//TODO:  if there is more than one interface, which one should be the source?
							strcpy(pkt.src, ifaces[0].macaddr);
							strcpy(pkt.dst, dstMac);
							
							cout << "ifaces[0] macaddr: " << ifaces[0].macaddr << endl;

							// TODO:pkt.type is determined based on whether or not we need to look up a MAC address
							// in the ARP cache
							pkt.type = 1;
							pkt.size = data.length();
							
							strcpy(pkt.data, data.c_str());
							vector<unsigned char> outBytes = writeEthernetPacketToBytes(pkt);
							
							char outbuf[BUFSIZE];
							memcpy(&outbuf, &outBytes[0], outBytes.size());
							
							cout << "outbuf: " << outbuf << endl;
							
							// TODO:  Which bridge should stdin be sent on?
							if(send(sockFd[0], outbuf, outBytes.size(), 0) <= 0)
								perror("send()");
						}

						else if(strncmp(buf, "show", 4) == 0 || strncmp(buf, "Show", 4) == 0) {
							//show whichever file it indicates in buf
						}
						else if(strncmp(buf, "Quit", 4) == 0 || strncmp(buf, "quit", 4) == 0) {
							exit(0);
						}
						//catch for invalid commands
						else {
							cout << "Invalid command: " << buf << endl;
						}
					}
				}
			}
		}
	}

	/* monitoring input from users and bridges
	* 1. from user: analyze the user input and send to the destination if necessary
	* 2. from bridge: check if it is for the station. Note two types of data
	* in the ethernet frame: ARP packet and IP packet.
	*
	* for a router, it may need to forward the IP packet
	*/
	
	return 0;
}



