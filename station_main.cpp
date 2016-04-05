/*-------------------------------------------------------*/

#include "parser.h"
#include <unistd.h>
#include "mac_layer.h"
#include "userinterface.h"
#include "station.h"

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
	bool routerFlag;
	
	if(strcmp(argv[1], "-router") < 0)
		routerFlag = true;
	else if(strcmp(argv[1], "-no") < 0)
		routerFlag = false;
	else {
		cout << "Router flag must be `-no` or `-station`" << endl;
		exit(1);
	}

	string ifaceFile(argv[2]);
	string rtableFile(argv[3]);
	string hostFile(argv[4]);
	
	Station station(routerFlag, ifaceFile, rtableFile, hostFile);
	
	// For debugging, dump everything associated with our station
	station.displayInterfaces();
	station.displayRouteTable();
	station.displayHostMap();
	

	/* hook to the lans that the station should connected to
	* note that a station may need to be connected to multilple lans
	*/
	
	fd_set masterSet;
	FD_ZERO(&masterSet);
	FD_SET(fileno(stdin), &masterSet);
	int maxFd = 0;
	
	station.connectToBridge();
	
	// If we can't connect to a bridge, then we exit.  Otherwise, keep track of
	// the socket and keep going.
	if(station.closed()) {
		cout << "Could not connect to any bridges.  Exiting." << endl;
		return 1;
	}
		
	FD_SET(station.socket(), &masterSet);
	
	// Redundant check; station.socket() should ALWAYS be greater than maxFd by this point
	if(station.socket() > maxFd)
		maxFd = station.socket();
	
	while(true) {
		fd_set readSet = masterSet;
		select(maxFd+1, &readSet, NULL, NULL, NULL);
		
		// TODO: cycle through pending queue and send out all possible packets
		station.sendPendingPackets();
		
		for(int i = 0; i <= maxFd; ++i) {
			int bytesRead = 0;
			char buf[BUFSIZE/2];
			memset(buf, '\0', sizeof buf);
			
			if (FD_ISSET(i, &readSet)) {
				if(i == station.socket()) {
					// If no bytes are read, something is wrong.  Exit.
					if((bytesRead = recv(i, buf, sizeof buf, 0)) <= 0) {
						cout << "recv error in select() loop" << endl;
						return 1;
					}
					//buf[bytesRead + 1] = '\0';
					//cout << ">>> " << buf;
					
					station.handlePacket(buf);
				}
				
				else if(i == fileno(stdin)) {
					bytesRead = read(i, buf, sizeof buf);
					
					if(bytesRead > 0) {
						// Need to make sure to terminate this for proper copying
						buf[bytesRead + 1] = '\0';
						
						station.handleUserInput(buf);
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



