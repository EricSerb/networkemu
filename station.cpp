/*-------------------------------------------------------*/
#include <arpa/inet.h>
 
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include "string_utils.h"
#include <iomanip>
#include "ip.h"

using namespace std;
/*----------------------------------------------------------------*/

void dumpInterfaces(vector<iface> ifaces)
{
	for(int i = 0; i < ifaces.size(); ++i) {
		cout << "Name: " << ifaces[i].ifacename << endl;
		cout << "IP Address (network byte order): " << ifaces[i].ipaddr << endl;
		cout << "Subnet Mask (network byte order): " << ifaces[i].mask << endl;
		cout << "Mac Address: ";
		for (int j = 0; j < 6; j++)
			cout << setfill('0') << setw(2) << hex << static_cast<int>(ifaces[i].macaddr[j]) << ":";
		cout << endl;
		cout << "Lan Name: " << ifaces[i].lanname << endl;
		cout << endl;
	}
}

vector<iface> extractInterfaces(string fn)
{
	// Test parsing interface file
	ifstream ifaceFile(fn.c_str());
	
	if(!ifaceFile.is_open()) {
		cout << "Error opening interface file." << endl;
		exit(1);
	}
	
	//vector< vector<string> > lines;
	vector<iface> ifaces;

	// Parse interface file, which is split by whitespaces
	string line;
	while(getline(ifaceFile, line)) {
		cout << __LINE__ << endl;
		stringstream linestream(line);
		string name;
		string ip;
		string subnet;
		string mac;
		string lan;
		linestream >> name >> ip >> subnet >> mac >> lan;
		cout << name << " " << ip << " " << subnet << " "  << mac << " "  << lan << endl;
		
		iface interface;
		strcpy(interface.ifacename, name.c_str());
		
		struct sockaddr_in sa;
		inet_pton(AF_INET, ip.c_str(),&(sa.sin_addr));
		interface.ipaddr = sa.sin_addr.s_addr;
		
		inet_pton(AF_INET, subnet.c_str(),&(sa.sin_addr));
		interface.mask = sa.sin_addr.s_addr;
		

		unsigned int iMac[6];
		int i;

		sscanf(mac.c_str(), "%x:%x:%x:%x:%x:%x", &iMac[0], &iMac[1], &iMac[2], &iMac[3], &iMac[4], &iMac[5]);
		for(i = 0;i < 6;i++)
			interface.macaddr[i] = (unsigned char)iMac[i];

			
		
		strcpy(interface.lanname, lan.c_str());
		
		ifaces.push_back(interface);
	}
	
	return ifaces;
}

/*----------------------------------------------------------------*/
/* station : gets hooked to all the lans in its ifaces file, sends/recvs pkts */
/* usage: station <-no -route> interface routingtable hostname */
int main (int argc, char *argv[])
{
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
	
	return 0;
}



