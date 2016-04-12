#ifndef STATION_H
#define STATION_H

#include <vector>
#include <map>
#include <string>
#include <sys/time.h>

#include "ip.h"
#include "ether_packet.h"
#include "arp_packet.h"
#include "ip_mac_interface.h"
#include "ip_packet.h"

struct CacheEntry {
	MacAddr mac;
	timeval timeStamp;
};

struct SocketBufferEntry {
	int fd;
	std::vector<unsigned char> bytes;
};

class Station {
public:
	Station(bool routerFlag, std::string ifaceFile, std::string rtableFile, std::string hostFile);
	
	bool router();
	
	std::vector<int> sockets();
	bool isSocket(int fd);
	void connectToBridge();
	
	void sendPendingPackets();
	SocketBufferEntry createSbEntry(IPAddr ip, std::vector<unsigned char> bytes);
	
	IPAddr ip();
	std::string mac();
	
	void handleUserInput(char inputBuffer[BUFSIZE]);
	void handlePacket(char inputBuffer[BUFSIZE], int incomingFd);
	IPAddr getNextHop(char ifacename[]);
	
	void moveFromArpWaitToPQ(ARP_PKT arpPkt);
	
	void constructArpRequest(IPAddr dstip);
	void constructArpReply(ARP_PKT general);
	void arpCacheTimeout();
	
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
	std::vector<int> m_fd; // If we have an open connection, it will be on this fd
	
	std::vector<iface> m_ifaces; // All interfaces attached to the station
	std::vector<rtable> m_rTableEntries; // Entries for the routing table
	std::map<std::string, IPAddr> m_hostMap; // Our DNS, maps hostname => IP
	std::vector<SocketBufferEntry> m_pendingQueue; // packets waiting to be sent out with KNOWN dest mac
	std::vector<EtherPkt> m_arpWaitQueue; // packets that can't be sent out until we know dest mac
	std::map<IPAddr, CacheEntry> m_arpCache; // map of IP address to to MAC address/time stamp
	std::map<IPAddr, int> m_fdLookup; //map of IPAddr to fd to use for routing table
	
};

#endif
