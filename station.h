#ifndef STATION_H
#define STATION_H

#include <vector>
#include <map>
#include <string>
#include "ip.h"
#include "ip_layer.h"
#include <sys/time.h>

struct CacheEntry {
	MacAddr mac;
	timeval timeStamp;
};

class Station {
public:
	Station(bool routerFlag, std::string ifaceFile, std::string rtableFile, std::string hostFile);
	
	bool router();
	
	int socket();
	void connectToBridge();
	void close();
	bool closed();
	
	void sendPendingPackets();
	
	IPAddr ip();
	std::string mac();
	
	void handleUserInput(char inputBuffer[BUFSIZE]);
	
	void constructArpRequest(IP_PKT ipPkt);
	
	void displayArpCache();
	void displayPQ();
	void displayInterfaces();
	void displayRouteTable();
	void displayHostMap();

	void insertArpCache(IPAddr ip, MacAddr mac);
	//Return the structure since you cannot return an array in c++....
	CacheEntry lookupArpCache(IPAddr ip);	
private:
	bool m_router; // Are we a router?
	
	// TODO: support more than one fd (i.e., more than one interface)
	int m_fd; // If we have an open connection, it will be on this fd
	
	std::vector<iface> m_ifaces; // All interfaces attached to the station
	std::vector<rtable> m_rTableEntries; // Entries for the routing table
	std::map<std::string, IPAddr> m_hostMap; // Our DNS, maps hostname => IP
	std::vector<std::vector<unsigned char > > m_pendingQueue; // packets waiting to be sent out with KNOWN dest mac
	std::vector<EtherPkt> m_arpWaitQueue; // packets that can't be sent out until we know dest mac
	std::map<IPAddr, CacheEntry> m_arpCache; // map of IP address to to MAC address/time stamp
	
};

#endif
