/*-------------------------------------------------------*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "string_utils.h"

using namespace std;
/*----------------------------------------------------------------*/


/*----------------------------------------------------------------*/
/* station : gets hooked to all the lans in its ifaces file, sends/recvs pkts */
/* usage: station <-no -route> interface routingtable hostname */
int main (int argc, char *argv[])
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
	/* initialization of hosts, interface, and routing tables */

	/* hook to the lans that the station should connected to
	* note that a station may need to be connected to multilple lans
	*/

	/* monitoring input from users and bridges
	* 1. from user: analyze the user input and send to the destination if necessary
	* 2. from bridge: check if it is for the station. Note two types of data
	* in the ethernet frame: ARP packet and IP packet.
	*
	* for a router, it may need to forward the IP packet
	*/
}



