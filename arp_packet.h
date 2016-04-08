#ifndef ARP_PACKET_H
#define ARP_PACKET_H

#include "ip.h"

class ARP_PKT{
public:
	ARP_PKT();
	void dump();

	short op; /* op =0 : ARP request; op = 1 : ARP response */
	IPAddr srcip;
	MacAddr srcmac;
	IPAddr dstip;
	MacAddr dstmac;
	
}__attribute__((packed));

#endif