#include "maclayer.h"
#include <iostream>
#include <cstring>

using namespace std;

/**
 * Deconstruct an EtherPkt and write it to a byte vector, which is
 * used so that we can write an EtherPkt byte by byte across the wire
 * (rather than trying to memcpy the struct)
 */
std::vector< unsigned char > writeEthernetPacketToBytes(EtherPkt pkt)
{
	vector<unsigned char> bytes;
	
	// First, add the destination mac addr.  size - 1 because final byte is null
	// terminator.  This will have to be added in at the other side
	for(unsigned int i = 0; i < sizeof(pkt.dst) - 1; ++i)
		bytes.insert(bytes.end(), pkt.dst[i]);
	
	// Add the source mac addr
	for(unsigned int i = 0; i < sizeof(pkt.src) - 1; ++i)
		bytes.insert(bytes.end(), pkt.src[i]);
	
	// Add the type.  Must add byte by byte
	short type = pkt.type;
	cout << "type: ";
	
	// Add the data (which is any valid string that is terminated by \0)
	for(unsigned int i = 0; i < sizeof(pkt.data); ++i) {
		if(pkt.data[i] == '\0')
			break;
		
		bytes.insert(bytes.end(), pkt.data[i]);
	}
	
	return bytes;
}
