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
#include "ether.h"

using namespace std;
/*----------------------------------------------------------------*/

/**
 * Convert an IPAddr to its printable form.
 */
string ntop(IPAddr addr)
{
	struct sockaddr_in sa;
	sa.sin_addr.s_addr = addr;
	
	char str[INET_ADDRSTRLEN];
	
	inet_ntop(AF_INET, &(sa.sin_addr), str, INET_ADDRSTRLEN);
	
	return str;
}

/**
 * Display all entries found in a given iface vector.
 */
void dumpInterfaces(vector<iface> ifaces)
{
	cout << "INTERFACES" << endl;
	cout << "NAME\tIP\tSUBNET\tMAC" << endl;
	for(unsigned int i = 0; i < ifaces.size(); ++i) {
		cout << ifaces[i].ifacename << "\t";
		cout << ntop(ifaces[i].ipaddr) << "\t";
		cout << ntop(ifaces[i].mask) << "\t";
		
		for (int j = 0; j < 6; j++) {
			cout << setfill('0') << setw(2) << hex << static_cast<int>(ifaces[i].macaddr[j]);
			if(j < 5)
				cout << ":";
			if(j == 5)
				cout << "\t";
		}

		cout << ifaces[i].lanname;
		cout << endl;
	}
	cout << endl;
}

/**
 * Display all entries found in a given rtable vector.
 */
void dumpRtables(vector<rtable> entries)
{
	cout << "ROUTING TABLE" << endl;
	cout << "DESTINATION\tNEXT HOP\tMASK\tINTERFACE" << endl;
	for (unsigned int i = 0; i < entries.size(); ++i) {
		cout << ntop(entries[i].destsubnet) << "\t";
		cout << ntop(entries[i].nexthop) << "\t";
		cout << ntop(entries[i].mask) << "\t";
		cout << entries[i].ifacename;
		cout << endl;
	}
	cout << endl;
	
}

/**
 * Display host information
 */
void dumpHosts(vector<Host> hosts)
{
	cout << "HOST INFORMATION" << endl;
	cout << "HOSTNAME\tIP ADDRESS\tPORT" << endl;
	for(unsigned int i = 0; i < hosts.size(); ++i) {
		cout << hosts[i].name << "\t";
		cout << ntop(hosts[i].addr) << "\t";
		cout << hosts[i].port << endl;
	}
	cout << endl;
}	

/**
 * Parse an interface file.
 */
vector<iface> extractInterfaces(string fn)
{
	// Test parsing interface file
	ifstream ifaceFile(fn.c_str());
	
	if(!ifaceFile.is_open()) {
		cout << "Error opening " << fn << endl;
		exit(1);
	}
	
	//vector< vector<string> > lines;
	vector<iface> ifaces;

	// Parse interface file, which is split by whitespaces
	string line;
	while(getline(ifaceFile, line)) {
		stringstream linestream(line);
		string name;
		string ip;
		string subnet;
		string mac;
		string lan;
		linestream >> name >> ip >> subnet >> mac >> lan;
		
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

/**
 * Parse a routing table file.
 */
vector<rtable> extractRouteTable(string fn)
{
	ifstream routeFile(fn.c_str());
	
	if(!routeFile.is_open()) {
		cout << "Error opening " << fn << endl;
		exit(1);
	}
	
	vector<rtable> entries;
	string line;
	
	while(getline(routeFile, line)) {
		stringstream linestream(line);
		string dest;
		string nexthop;
		string mask;
		string ifacename;
		linestream >> dest >> nexthop >> mask >> ifacename;
		
		rtable entry;
		
		struct sockaddr_in sa;
		inet_pton(AF_INET, dest.c_str(), &(sa.sin_addr));
		entry.destsubnet = sa.sin_addr.s_addr;
		
		inet_pton(AF_INET, nexthop.c_str(), &(sa.sin_addr));
		entry.nexthop = sa.sin_addr.s_addr;
		
		inet_pton(AF_INET, mask.c_str(), &(sa.sin_addr));
		entry.mask = sa.sin_addr.s_addr;
		
		strcpy(entry.ifacename, ifacename.c_str());
		
		entries.push_back(entry);
	}
	
	return entries;
}

/**
 * Parse a hostfile.  This file acts as our DNS lookup table, and can
 * contain multiple hosts.
 */
vector<Host> extractHosts(string fn)
{
	ifstream hostFile(fn.c_str());
	
	if(!hostFile.is_open()) {
		cout << "Error opening " << fn << endl;
		exit(1);
	}
	
	string line;
	vector<Host> hosts;
	//TODO: probably should do some error checking to make sure host is valid
	while(getline(hostFile, line)) {
		stringstream linestream(line);
		
		string name;
		string addr;
		int port;
		
		linestream >> name >> addr >> port;
		
		Host h;
		
		strcpy(h.name, name.c_str());
		
		sockaddr_in sa;
		inet_pton(AF_INET, addr.c_str(), &(sa.sin_addr));
		h.addr = sa.sin_addr.s_addr;
		
		h.port = port;
		hosts.push_back(h);
	}
	
	return hosts;
}

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

	/* monitoring input from users and bridges
	* 1. from user: analyze the user input and send to the destination if necessary
	* 2. from bridge: check if it is for the station. Note two types of data
	* in the ethernet frame: ARP packet and IP packet.
	*
	* for a router, it may need to forward the IP packet
	*/
	
	return 0;
}



