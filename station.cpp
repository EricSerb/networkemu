/*-------------------------------------------------------*/

#include "parser.h"

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
	
	// This is essentially our DNS lookup table
	vector<Host> hosts = extractHosts(fn);
	
	dumpHosts(hosts);

	/* hook to the lans that the station should connected to
	* note that a station may need to be connected to multilple lans
	*/
	/*
	struct addrinfo hints;
	memset(&hints, 0, sizeof hints);
	
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	struct addrinfo *res;
	
	if(argc < 3) {
		cout << "Usage: ./chatclient <host> <port>" << endl;
		return 1;
	}
	
	getaddrinfo(argv[1], argv[2], &hints, &res);
	
	int sockFd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	
	if(connect(sockFd, res->ai_addr, res->ai_addrlen) < 0) {
		perror("connect()");
		return -1;
	}
	
	cout << "Connected to server!" << endl;

	fd_set masterSet;
	FD_ZERO(&masterSet);
	FD_SET(fileno(stdin), &masterSet);
	FD_SET(sockFd, &masterSet);
	
	int maxFd = sockFd;
	
	while(true) {
		fd_set readSet = masterSet;
		select(maxFd+1, &readSet, NULL, NULL, NULL);
		
		int bytesRead = 0;
		char buf[500];
		memset(buf, '\0', sizeof buf);
		
		for(int i = 0; i <= maxFd; ++i) {
			if (FD_ISSET(i, &readSet)) {
				if(i == sockFd) {
					// If no bytes are read, something is wrong.  Exit.
					if((bytesRead = recv(i, buf, sizeof buf, 0)) <= 0) {
						cout << "recv error" << endl;
						return 1;
					}
					
					cout << ">>> " << buf;
				}
				
				else if(i == fileno(stdin)) {
					bytesRead = read(i, buf, sizeof buf);
					
					if(bytesRead > 0) {
						if(send(sockFd, buf, bytesRead, 0) <= 0)
							perror("send()");
					}
				}
			}
		}
	}*/

	/* monitoring input from users and bridges
	* 1. from user: analyze the user input and send to the destination if necessary
	* 2. from bridge: check if it is for the station. Note two types of data
	* in the ethernet frame: ARP packet and IP packet.
	*
	* for a router, it may need to forward the IP packet
	*/
	
	return 0;
}



