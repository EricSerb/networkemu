#ifndef IP_PACKET_H
#define IP_PACKET_H

#include "ip.h"

class IP_PKT
{
public:
	IP_PKT();
	void dump();
	
	IPAddr  dstip;
	IPAddr  srcip;
	short   length;
	char    data[BUFSIZE-(ETHPKTHEADER+IPPKTHEADER)];
};

#endif