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
		bytes.push_back(pkt.dst[i]);
	
	// Add the source mac addr
	for(unsigned int i = 0; i < sizeof(pkt.src) - 1; ++i)
		bytes.push_back(pkt.src[i]);
	
	// Add the type.  We cannot directly add the bytes of a short to the vector,
	// so cast it to a string first
	string type = to_string((int)pkt.type);
	
	for(unsigned int i = 0; i < type.length(); ++i)
		bytes.push_back(type[i]);
	
	// Size is also a short and must be read byte by byte.
	string size = to_string((int)pkt.size);
	// TODO: pkt.size represents how full a data buffer is.  This could be
	// 1 - 4 digits long (max of 1024), so for all size strings less than 4 characters,
	// maybe we should pad it up to 4 characters (precede the size with 0's).  That way,
	// we can just extract 4 characters on the receivers end.  This lets us know where the 'size'
	// field ends and where the data buffer begins
	for(unsigned int i = 0; i < size.length(); ++i)
		bytes.push_back(size[i]);
	
	// Add the data (which is any valid string that is terminated by \0)
	for(unsigned int i = 0; i < sizeof(pkt.data); ++i) {
		if(pkt.data[i] == '\0')
			break;
		
		bytes.push_back(pkt.data[i]);
	}
	
	return bytes;
}
