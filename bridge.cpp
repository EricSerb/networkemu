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
/*----------------------------------------------------------------*/

using namespace std;

/**
 * The structure to be included as the value for the
 * self learn map (port => MacTableEntry)
 */
struct MacTableEntry
{
	int port;
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
	//buff for client interaction
	char buf[1024];
	int yes = 1;
	int addrlen;
	int i, j;
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
	queue<PacketQ> recPackets;

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

	for(;;)
	{
		memset(&buf, 0, sizeof(buf));
		read_fds = master;

		//Get the current time and then erase entries in the map if they are older than 30 seconds
		timeval currentTime;
		gettimeofday(&currentTime, NULL);

		
		for(auto &it : selfLearnTable) {
			if((currentTime.tv_sec - it.second.timeStamp.tv_sec) > 30) {
				selfLearnTable.erase(it.first);
			}
		}
		
		//Send out all the packets in the Queue
		while(!recPackets.empty())
		{
			PacketQ toSend = recPackets.front();
			recPackets.pop();
			/* Send out packets here
			 * If next hop is not known then broadcast using for loop code
			 */
			if(!toSend.known)
			{
				//This is the code for sending out to all ports except the one received on
				for(j = 0; j <= fdmax; j++)
				{
					//send to everyone
					if(FD_ISSET(j, &master))
					{
						//except the listener and server and port sent in on
						if(j != listener && j != i && j != toSend.ownport)
						{
							//cout << buf << " i = " << j << endl;
							if(send(j, &toSend.buf[0], sizeof(toSend.buf), 0) == -1)
							{
								cout << "Bridge send() error..." << endl;
							}
						}
					}
				}
			}
			else //we know the next hop just need to send to that port
			{
				if(send(toSend.port, &toSend.buf[0], sizeof(toSend.buf), 0) == -1)
				{
					cout << "Bridge send() error..." << endl;
				}
			}
		}

		if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
		{
			cout << "Bridge select() error... CRASH!" << endl;
			exit(1);
		}
		//cout << "Server select() ok..." << endl;

		//check existing connections for anything to read
		for(i = 0; i <= fdmax; i++)
		{
			if(FD_ISSET(i, &read_fds))
			{
				if(i == listener)
				{
					//handle new connections
					addrlen = sizeof(caddr);
					if((newfd = accept(listener, (struct sockaddr *)&caddr, (socklen_t *) &addrlen)) == -1)
					{
						cout << "Bridge accpet() error...." << endl;
					}
					else
					{
						if(portCount >= numPorts)
						{
							strcpy(buf, "Reject");
							nbytes = 7;
							send(newfd, buf, nbytes, 0);
						}
						else
						{
							FD_SET(newfd, &master); //adding to master set
							portCount++; //increase port count to keep track of number of ports in use
							if(newfd > fdmax)
							{
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
				else
				{
					//handle data from client
					if((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0)
					{
						//error or connection closed
						if(nbytes == 0)
						{
							cout << argv[0] << ": socket " << i << " hung up" << endl;
						}
						else
						{
							cout << "Bridge recv() error..." << endl;
						}

						portCount--; //decrease count since client no longer using port
						close(i);
						FD_CLR(i, &master);

					}
					else
					{	
						PacketQ pkt;
						
					/*	
						// If the port isn't in the self-learn table, then add it
						if(selfLearnTable.find(peerAddr.sin_port) == selfLearnTable.end()) {
							MacTableEntry entry;
							// &buf[6] should refer to the MacAddr src field of Ethernet Pkt
							memcpy(&(entry.mac), &buf[6], 6);
							gettimeofday(&entry.timeStamp, NULL);
							selfLearnTable.insert(pair<unsigned long, MacTableEntry>(peerAddr.sin_port, entry));
						}
					*/
					
						//Cout for testing that we are receiving packets correctly
						//cout << ">>" << buf << endl << endl;

						strcpy(pkt.buf, buf);
						string src;
						memcpy(&src, &buf[17], 17);

						// Determine which port the packet came in on
						struct sockaddr_in peerAddr;
						socklen_t peerAddrLen = sizeof peerAddr;
						getpeername(i,  (sockaddr*) &peerAddr, &peerAddrLen);

						pkt.ownport = peerAddr.sin_port;
						//If the MacAddr is not in the self-learn table, then add it
						if(selfLearnTable.find(src) == selfLearnTable.end())
						{
							//Create the MacTableEntry to hold the port and timeStamp
							MacTableEntry entry;
							entry.port = peerAddr.sin_port;
							gettimeofday(&entry.timeStamp, NULL);
							selfLearnTable.insert(pair<string, MacTableEntry>(src, entry));
						}

						
						cout << "*******Testing the self learn table*******" << endl;
						for(auto &it : selfLearnTable)
						{
							cout << "Mac == " << it.first << endl;
							cout << "Port == " << it.second.port << endl << endl;
						}
						cout << "*******End self learn table testing*******" << endl;
						// Lookup destination mac address in self learn table to see if port is known
						string dest;
						memcpy(&dest, &buf[17], 17);
						
						// If the destination is in the self learn  table, make sure the PacketQ known
						// value gets set to true, so that we do not broadcast it
						for(auto &it : selfLearnTable) {
							if(it.first == dest) {
								pkt.known = true;
								pkt.port = it.second.port;
								break;
							}
						}
						
						recPackets.push(pkt);

					}
				}
			}
		}
	}
}
/*----------------------------------------------------------------*/
