#ifndef ETHER_PACKET_H
#define ETHER_PACKET_H

#include "ip.h"

/* structure of an ethernet pkt
 * We can copy the contents of an IP_PKT data buffer to this data buffer
 * when it is time for a station to construct a packet to send via MAC address
 */
class EtherPkt 
{
public:
	EtherPkt();
	
	void dump();
	
	/* destination address in net order */
	MacAddr dst;

	/* source address in net order */
	MacAddr src;

	/************************************/
	/* payload type in host order       */
	/* type = 0 : ARP frame             */
	/* type = 1 : IP  frame             */
	/************************************/
	short  type;

	/* size of the data in host order */
	short   size;

	/* actual payload */
	char data[ETHBUFSIZE];

};

#endif