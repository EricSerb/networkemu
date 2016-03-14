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
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "string_utils.h"
/*----------------------------------------------------------------*/

using namespace std;

/* bridge : recvs pkts and relays them */
/* usage: bridge lan-name max-port */
int 
main (int argc, char *argv[])
{
	// Test parsing interface file
	ifstream ifaceFile("./ifaces/ifaces.a");
	
	if(!ifaceFile.is_open()) {
		cout << "Error opening interface file." << endl;
		exit(1);
	}
	
	vector< vector<string> > ifaces;
	
	while(!ifaceFile.eof()) {
		string iface;
		getline(ifaceFile, iface);
		ifaces.push_back(split(iface, ' '));
	}
	
	for(int i = 0; i < ifaces.size(); ++i) {
		for(int j = 0; j < ifaces[i].size(); ++j) {
			cout << ifaces[i][j] << " ";
		}
		cout << endl;
	}
	
	/* create the symbolic links to its address and port number
	* so that others (stations/routers) can connect to it
	*/

	/* listen to the socket.
	* two cases:
	* 1. connection open/close request from stations/routers
	* 2. regular data packets
	*/
}
/*----------------------------------------------------------------*/
