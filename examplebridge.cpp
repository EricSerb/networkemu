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
#include <string.h>
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
#include <map>
#include <sys/time.h>
#include "ip.h"
/*----------------------------------------------------------------*/

/**
 * The structure to be included as the value for the
 * self learn map (port => MacTableEntry)
 */
struct MacTableEntry
{
	MacAddr mac;
	timeval timeStamp;
};

using namespace std;

/* bridge : recvs pkts and relays them */
/* usage: bridge lan-name max-port */
int 
main (int argc, char *argv[])
{	
	/* create the symbolic links to its address and port number
	* so that others (stations/routers) can connect to it
	*/

	/* listen to the socket.
	* two cases:
	* 1. connection open/close request from stations/routers
	* 2. regular data packets
	*/
	
	// The self learn table.  sin_port of is type unsigned short
	map<unsigned short, MacTableEntry> selfLearnTable;
	
	while(true) {
		if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
		}

		//check existing connections for anything to read
		for(int i = 0; i <= fdmax; i++) {
			if(/* Packet received on socket with file descriptor i */) {
				// Determine which port the packet came in on
				struct sockaddr_in peerAddr;
				socklen_t peerAddrLen = sizeof peerAddr;
				getpeername(i, &peerAddr, &peerAddrLen);
				
				// If the port isn't in the self-learn table, then add it
				if(selfLearnTable.find(peerAddr.sin_port) == selfLearnTable.end()) {
					MacTableEntry entry;
					entry.mac = 0; // mac address will have to be extracted from the ethernet packet
					gettimeofday(entry.timeStamp, NULL);
					selfLearnTable.insert(pair<unsigned long, MacTableEntry>(peerAddr.sin_port, entry);
				}
			}
		}
	}
}
/*----------------------------------------------------------------*/
