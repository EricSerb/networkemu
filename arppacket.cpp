#include "arppacket.h"

void ArpPacket::ArpPacket()
{
	srcip = 0;
	dstip = 0;
	
	for(int i = 0; i < 18; i++)
	{
		dstmac[i] = srcmac[i] = 0;
	}
}

 