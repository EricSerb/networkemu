#include "arp_packet.h"
#include <iostream>

using namespace std;

ARP_PKT::ARP_PKT()
{
	op = 0;
	srcip = 0;
	dstip = 0;
	
	for(unsigned int i = 0; i < sizeof(MacAddr); ++i)
		dstmac[i] = 0;
	
	for(unsigned int i = 0; i < sizeof(MacAddr); ++i)
		srcmac[i] = 0;
}

void ARP_PKT::dump()
{
	cout << "ARP PACKET DUMP" << endl;
	cout << "op: " << op << endl;
	cout << "srcip: " << srcip << endl;
	cout << "srcmac: " << srcmac << endl;
	cout << "dstip: " << dstip << endl;
	cout << "dstmac: " << dstmac << endl;
}