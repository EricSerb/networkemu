/*----------------------------------------------------------------*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>

//Eric's headers used in server.cpp
#include <stdlib.h>
#include <iostream>
#include <string>
#include <unistd.h>

//New headers
#include <queue>
#include <vector>
#include <fstream>
#include "ip.h"
#include <cstring>
#include <map>
#include "ether_packet.h"
/*----------------------------------------------------------------*/

using namespace std;

/**
 * The structure to be included as the value for the
 * self learn map (port => MacTableEntry)
 */
struct MacTableEntry
{
	int socket;
	timeval timeStamp;
};

/* bridge : recvs pkts and relays them */
/* usage: bridge lan-name max-port */
int main (int argc, char *argv[])
{	
	/* create the symbolic links to its address and port number
	* so that others (stations/routers) can connect to it
	*/

	/* listen to the socket.
	* two cases:
	* 1. connection open/close request from stations/routers
	* 2. regular data packets
	*/


	//max fd #, listening socket descriptor, newly accepted fd, # bytes in buff
	int fdmax, listener, newfd, nbytes;

	int yes = 1;
	int addrlen;
	int numPorts = atoi(argv[2]);
	int portCount = 0;
	string message;
	//server address
	struct sockaddr_in saddr;
	//client addr
	struct sockaddr_in caddr;
	//master fd list
	fd_set master;
	//temp fd list for select()
	fd_set read_fds;
	//char linprog2[] = {"128.186.120.34"};

	//Vector for routing table
	vector<rtable> rTable;

	//Queue for holding packets received and need to be sent out. Also variables for holding 
	//newly arrived packets so that they can be placed in the queue
	vector<PacketQ> recPackets;

	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	memset(&saddr, 0, sizeof(saddr));
	memset(&caddr, 0, sizeof(caddr));

	if(argc < 3)
	{
		cout << "Command line must include: .exe, lan-name, num-ports" << endl;
		cout << "Bridge failure..." << endl;
		exit(1);
	}

	//socket
	if((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		cout << "Bridge socket() error... CRASH!" << endl;
		exit(1);
	}

	cout << "Bridge socket() ok..." << endl;

	//addr already in use
	if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		cout << "Bridge setsockopt() error... CRASH!" << endl;
		exit(1);
	}

	cout << "Bridge setsockopt ok..." << endl;

	char temp[255];
	gethostname(temp, 255);
	struct hostent *host_entry;
	host_entry = gethostbyname(temp);
	memcpy(&saddr.sin_addr.s_addr, host_entry->h_addr, host_entry->h_length);

	//bind
	saddr.sin_family = AF_INET;
	//inet_aton(linprog2, &saddr.sin_addr);
	//memset(&(saddr.sin_zero), 0, 8);

	if(bind(listener, (struct sockaddr *)&saddr, sizeof(saddr)) == -1)
	{
		cout << "Bridge bind() error... CRASH!" << endl;
		exit(1);
	}
	//unsigned long p = saddr.sin_port;

	//cout << ntohl(p) << endl;

	
	//saddr.sin_port = p;
	cout << "Bridge bind() ok..." << endl;
	
	//cout << "Server address = " << saddr.sin_addr.s_addr << endl;
	//cout << "Server port number = " << saddr.sin_port << endl;

	if(listen(listener, 10) == -1)
	{
		cout << "Bridge listen() error... CRASH!" << endl;
		exit(1);
	}
	
	cout << "Bridge listen() ok..." << endl;
	
	socklen_t len = sizeof(saddr);
	if(getsockname(listener, (struct sockaddr*)&saddr, &len) == -1)
	{
		cout << "Bridge getsockname() error... CRASH!" << endl;
		exit(1);
	}
	//add listener to the master set
	FD_SET(listener, &master);
	
	//keeping track of biggest fd
	fdmax = listener; 

	cout << endl;

	//Write the IP addr and port number to files for station to use later
	string hostName;
	hostName.assign(argv[1]);
	ofstream hostFile;
	hostName += ".info";
	hostFile.open(hostName.c_str(), std::ofstream::out);
	
	hostFile << inet_ntoa(saddr.sin_addr) << " " <<  ntohs(saddr.sin_port);

	hostFile.close();
	//No longer need to put out name and address as stations will read this from a file
	/*
	cout << "Server name = " << temp << endl;
	cout << "Server address = " << inet_ntoa(saddr.sin_addr) << endl;
	cout << "Server port number = " << ntohs(saddr.sin_port) << endl;
	*/
	
	// The self learn table.  sin_port of is type unsigned short
	map<string, MacTableEntry> selfLearnTable;
	
	recPackets.clear();

	for(;;) {
		char buf[BUFSIZE];
		for(unsigned int i = 0; i < sizeof(buf); ++i)
			buf[i] = 0;

		read_fds = master;

		//Get the current time and then erase entries in the map if they are older than 30 seconds
		timeval currentTime;
		gettimeofday(&currentTime, NULL);

		
		for(auto &it : selfLearnTable) {
			if((currentTime.tv_sec - it.second.timeStamp.tv_sec) > 30) {
				cout << "Removing " << it.first << " from selfLearnTable" << endl;
				selfLearnTable.erase(it.first);
			}
		}
		
		// Send out all the packets in the Queue
		while(!recPackets.empty()) {
			PacketQ toSend = recPackets.back();
			recPackets.pop_back();
			
			/* Send out packets here
			 * If next hop is not known then broadcast using for loop code
			 */
			if(!toSend.known || toSend.arpType == ARP_REQUEST) {
				//This is the code for sending out to all ports except the one received on
				for(int i = 0; i <= fdmax; i++) {
					//send to everyone except the listener and server and port sent in on
					if(FD_ISSET(i, &master) && i != listener && i != toSend.socketIn) {
							cout << "i: " << i << endl;
							//cout << buf << " i = " << j << endl;
							if(send(i, &toSend.buf[0], sizeof(toSend.buf), 0) == -1) {
								cout << "Bridge send() error..." << endl;
							}
						}
					}
				}
			//we know the next hop just need to send to that port
			else {
				if(send(toSend.socketOut, &toSend.buf[0], sizeof(toSend.buf), 0) == -1) {
					cout << "Bridge send() error..." << endl;
				}
				
				cout << "Sent to " << toSend.socketOut << endl;
			}
		}
		timeval timeout;
		timeout.tv_sec = 0;
		if(select(fdmax+1, &read_fds, NULL, NULL, &timeout) == -1) {
			cout << "Bridge select() error... CRASH!" << endl;
			exit(1);
		}

		//check existing connections for anything to read
		for(int i = 0; i <= fdmax; i++) {
			if(FD_ISSET(i, &read_fds)) {
				if(i == listener) {
					//handle new connections
					addrlen = sizeof(caddr);
					if((newfd = accept(listener, (struct sockaddr *)&caddr, (socklen_t *) &addrlen)) == -1) {
						cout << "Bridge accpet() error...." << endl;
					}
					else {
						if(portCount >= numPorts) {
							strcpy(buf, "Reject");
							nbytes = 7;
							send(newfd, buf, nbytes, 0);
						}
						else {
							FD_SET(newfd, &master); //adding to master set
							portCount++; //increase port count to keep track of number of ports in use
							if(newfd > fdmax) {
								fdmax = newfd;
							}

							strcpy(buf, "Accept");
							nbytes = 7;
							send(newfd, buf, nbytes, 0);

							cout << endl << endl << argv[0] << ": New connection from " << inet_ntoa(caddr.sin_addr) << " on socket " << newfd << endl;

							cout << endl;
							cout << "Bridge name = " << temp << endl;
							cout << "Bridge address = " << inet_ntoa(saddr.sin_addr) << endl;
							cout << "Bridge port number = " << ntohs(saddr.sin_port) << endl;
						}
					}
				}
				else {
					//handle data from client
					if((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0) {
						//error or connection closed
						if(nbytes == 0) {
							cout << argv[0] << ": socket " << i << " hung up" << endl;
						}
						else {
							cout << "Bridge recv() error..." << endl;
						}

						portCount--; //decrease count since client no longer using port
						close(i);
						FD_CLR(i, &master);

					}
					else {	
						PacketQ pkt;
					
						//Cout for testing that we are receiving packets correctly
						cout << "nbytes: " << nbytes << endl;
						strcpy(pkt.buf, buf);
						cout << "buf: " << buf << endl;
						string buffer;
						buffer.assign(buf);
						cout << "buffer: " << buffer << endl;
						string src;
						src.assign(buffer, 17, 17);
						//cout << "src: " << src << endl;
						//memcpy(&src, &buf[17], 17);
						
						cout << src << "==" << buf << endl << endl;

						// Determine which port the packet came in on
						//struct sockaddr_in peerAddr;
						//socklen_t peerAddrLen = sizeof peerAddr;
						//getpeername(i,  (sockaddr*) &peerAddr, &peerAddrLen);

						pkt.socketIn = i;
						string type;
						type.clear();
						//this gets the packet type
						type.assign(buffer, 36, 2);
						short packetType = atoi(type.c_str());
						type.clear();
						
						//Now we will get the arp type if packet type is arp
						if(packetType == TYPE_ARP_PKT) {
							//Will set arpType to request or reply
							type.assign(buffer, ETHPKTHEADER, 2);
							pkt.arpType = atoi(type.c_str());
						}
						else
							pkt.arpType = -1; //no arp type since it is an ip packet
						
						
						//If the MacAddr is not in the self-learn table, then add it
						if(selfLearnTable.find(src) == selfLearnTable.end()) {
							cout << "Adding " << src << " to self learn table" << endl;
							//Create the MacTableEntry to hold the port and timeStamp
							MacTableEntry entry;
							entry.socket = i;
							gettimeofday(&entry.timeStamp, NULL);
							selfLearnTable.insert(pair<string, MacTableEntry>(src, entry));
						}
						// We know it exists, so update the timestamp
						else {
							auto it = selfLearnTable.find(src);
							gettimeofday(&it->second.timeStamp, NULL);
						}

						// Lookup destination mac address in self learn table to see if port is known
						string dest;
						dest.assign(buffer, 0, 17);
						
						// If the destination is in the self learn  table, make sure the PacketQ known
						// value gets set to true, so that we do not broadcast it
						for(auto &it : selfLearnTable) {
							if(it.first == dest) {
								cout << dest << " is already in the selfLearnTable.  Updating time stamp" << endl;
								pkt.known = true;
								pkt.socketOut = it.second.socket;
								break;
							}
						}
						
						recPackets.push_back(pkt);

					}
				}
			}
		}
	}
}
/*----------------------------------------------------------------*/
