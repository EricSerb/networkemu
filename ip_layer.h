#ifndef IP_LAYER_H
#define IP_LAYER_H

#include "ip.h"
#include <map>

class ARPcache {
public:
	void displayTable();
private:
	std::map<IPAddr, MacAddr> m_ipToMacTable;
};

#endif