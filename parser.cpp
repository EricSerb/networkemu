#include "parser.h"

using namespace std;

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

		// Be sure to terminate the macaddr, lest we get strange errors..
		strcpy(interface.macaddr, mac.c_str());
		interface.macaddr[17] = '\0';

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
map<string, IPAddr> extractHosts(string fn)
{
	ifstream hostFile(fn.c_str());
	
	if(!hostFile.is_open()) {
		cout << "Error opening " << fn << endl;
		exit(1);
	}
	
	string line;
	map<string, IPAddr> hosts;
	//TODO: probably should do some error checking to make sure host is valid
	while(getline(hostFile, line)) {
		stringstream linestream(line);
		
		string name;
		string addr;
		
		linestream >> name >> addr;
		
		Host h;
		
		h.name = name;
		
		sockaddr_in sa;
		inet_pton(AF_INET, addr.c_str(), &(sa.sin_addr));
		h.addr = sa.sin_addr.s_addr;
		
		hosts.insert(pair<string, IPAddr>(h.name, h.addr));
		
	}
	
	return hosts;
}