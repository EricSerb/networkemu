/*-------------------------------------------------------*/

#include "parser.h"
#include <unistd.h>
#include "ip_mac_interface.h"
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
	
	if(strcmp(argv[1], "-router") == 0)
		routerFlag = true;
	else if(strcmp(argv[1], "-no") == 0)
		routerFlag = false;
	else {
		cout << "Router flag must be `-no` or `-router`" << endl;
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
	vector<int> fds = station.sockets();
	if(fds.size() == 0) {
		cout << "Could not connect to any bridges.  Exiting." << endl;
		return 1;
	}
	
	for(unsigned int i = 0; i < fds.size(); ++i) {
		FD_SET(fds[i], &masterSet);
		if(fds[i] > maxFd)
			maxFd = fds[i];
	}
	
	while(true) {
		fd_set readSet = masterSet;
		timeval timeout;
		timeout.tv_sec = 0;
		select(maxFd+1, &readSet, NULL, NULL, &timeout);

		station.sendPendingPackets();
		station.arpCacheTimeout();
		for(int i = 0; i <= maxFd; ++i) {
			int bytesRead = 0;
			
			char buf[BUFSIZE];
			for(unsigned int j = 0; j < sizeof(buf); ++j)
				buf[j] = 0;

			if (FD_ISSET(i, &readSet)) {
				if(station.isSocket(i)) {
					// If no bytes are read, something is wrong.  Exit.
					if((bytesRead = recv(i, buf, sizeof(buf), 0)) <= 0) {
						cout << "recv error in select() loop" << endl;
						return 1;
					}
					cout << "received packet from bridge (socket " << i << ") with bytesRead: " << bytesRead << " and buf: " << buf << endl;
					
					station.handlePacket(buf, i);
				}
				
				else if(!station.router() && i == fileno(stdin)) {
					bytesRead = read(i, buf, sizeof(buf));

					if(bytesRead > 0) {
						cout << "before calling handleUserInput(), our buf is: " << buf << endl;
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



