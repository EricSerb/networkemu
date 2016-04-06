#include "ip_mac_interface.h"
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

EtherPkt writeBytesToEtherPacket(char *buffer)
{
	EtherPkt etherPkt;
	memcpy(&etherPkt.dst, &buffer[0], 18);
	memcpy(&etherPkt.src, &buffer[18], 18);
	memcpy(&etherPkt.type, &buffer[36], 2);
	memcpy(&etherPkt.size, &buffer[38], 2);
	memcpy(&etherPkt.data, &buffer[40], BUFSIZE/2);
	
	return etherPkt;
}


/**
 * Converts an ARP PKT to a byte vector, so that it can easily
 * be added to the pending queue of packets to be sent out.  Order matters here.
 * Must add bytes in the same order that we will deconstruct them in
 */
std::vector<unsigned char> writeArpPktToBytes(ARP_PKT pkt)
{
	vector<unsigned char> bytes;
	
	// Add the type.  We cannot directly add the bytes of a short to the vector,
	// so cast it to a string first
	string op = to_string((int)pkt.op);
	
	for(unsigned int i = 0; i < op.length(); ++i)
		bytes.push_back(op[i]);
	
	// Add the source mac addr
	string srcIP = to_string((int)pkt.srcip);
	
	for(unsigned int i = 0; i < srcIP.length(); ++i)
		bytes.push_back(srcIP[i]);
	
	// size - 1 because final byte is null terminator.
	// This will have to be added in at the other side
	for(unsigned int i = 0; i < sizeof(pkt.srcmac) - 1; ++i)
		bytes.push_back(pkt.srcmac[i]);
	
	string dstIP = to_string((int)pkt.dstip);
	
	for(unsigned int i = 0; i < dstIP.length(); ++i)
		bytes.push_back(dstIP[i]);
	
	// size - 1 because final byte is null terminator.
	// This will have to be added in at the other side
	for(unsigned int i = 0; i < sizeof(pkt.dstmac) - 1; ++i)
		bytes.push_back(pkt.dstmac[i]);
	
	return bytes;
}

ARP_PKT writeBytesToArpPkt(char* buffer)
{
	ARP_PKT pkt;
	
	// get the op out
	memcpy(&(pkt.op), &(buffer[0]), 2);
	
	//get the src ip
	memcpy(&(pkt.srcip), &(buffer[2]), 4);
	
	//get src mac
	memcpy(&(pkt.srcmac), &(buffer[6]), 18);
	
	memcpy(&(pkt.dstip), &(buffer[24]), 4);
	
	memcpy(&(pkt.dstmac), &(buffer[28]), 18);
	
	return pkt;
}

IP_PKT writeBytesToIpPkt(char *buffer)
{
	IP_PKT pkt;
	
	//memcpy areas of buffer to form the ip packet that was just sent to us
	memcpy(&(pkt.dstip), &(buffer[0]), 4);
	
	memcpy(&(pkt.srcip), &(buffer[4]), 4);
	
	memcpy(&(pkt.length), &(buffer[14]), 2);
	
	memcpy((pkt.data), &(buffer[16]), pkt.length);
	
	return pkt;
}



	
