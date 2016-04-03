#ifndef STATION_H
#define STATION_H

#include <vector>
#include <map>
#include <string>
#include "ip.h"

class Station {
public:
	Station(bool routerFlag, std::string ifaceFile, std::string rtableFile, std::string hostFile);
	
	bool router();
	
	int socket();
	void connectToBridge();
	void close();
	bool closed();
	
	void handleUserInput(char inputBuffer[BUFSIZE]);
	
	void displayInterfaces();
	void displayRouteTable();
	void displayHostMap();
private:
	bool m_router; // Are we a router?
	
	// TODO: support more than one fd (i.e., more than one interface)
	int m_fd; // If we have an open connection, it will be on this fd
	
	std::vector<iface> m_ifaces; // All interfaces attached to the station
	std::vector<rtable> m_rTableEntries; // Entries for the routing table
	std::map<std::string, IPAddr> m_hostMap; // Our DNS, maps hostname => IP
};

#endif