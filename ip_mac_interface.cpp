#include "ip_mac_interface.h"
#include <iostream>
#include <cstring>
#include "parser.h"

// number of bytes in a short.  used when padding values
// that do not fill all 4 bytes
#define NUM_BYTES_SHORT 4

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
	for(unsigned int i = 0; i < (NUM_BYTES_SHORT - type.length()); ++i)
		bytes.push_back('0');
	
	for(unsigned int i = 0; i < type.length(); ++i)
		bytes.push_back(type[i]);
	
	// Size is also a short and must be read byte by byte.
	string size = to_string((int)pkt.size);;

	for(unsigned int i = 0; i < (NUM_BYTES_SHORT - size.length()); ++i)
		bytes.push_back('0');
	
	for(unsigned int i = 0; i < size.length(); ++i)
		bytes.push_back(size[i]);
	
	// Add the data (which is any valid string that is terminated by \0)
	for(unsigned int i = 0; i < sizeof(pkt.data); ++i) {
		if(pkt.data[i] == 0)
			break;
		
		bytes.push_back(pkt.data[i]);
	}

	return bytes;
}

/**
 * Writes the bytes of an IP packet into a byte vector.  This is intended
 * to be used so that IP packets can be encapsulated in the data buffer of
 * an ethernet packet.  Order matters.  Consult ip.h for structure setup.
 */
std::vector< unsigned char > writeIpPktToBytes(IP_PKT pkt)
{
	vector<unsigned char> bytes;
	string dstIP = to_string((int)pkt.dstip);

	for(unsigned int i = 0; i < (sizeof(IPAddr) + 2 - dstIP.length()); ++i)
		bytes.push_back('0');
	
	for(unsigned int i = 0; i < dstIP.length(); ++i)
		bytes.push_back(dstIP[i]);
	
	string srcIP = to_string((int)pkt.srcip);
	for(unsigned int i = 0; i < (sizeof(IPAddr) + 2 - srcIP.length()); ++i)
		bytes.push_back('0');

	for(unsigned int i = 0; i < srcIP.length(); ++i)
		bytes.push_back(srcIP[i]);
	
	string len = to_string((int)pkt.length);
	for(unsigned int i = 0; i < (NUM_BYTES_SHORT - len.length()); ++i)
		bytes.push_back('0');
	
	for(unsigned int i = 0; i < len.length(); ++i)
		bytes.push_back(len[i]);
	
	// Add the data (which is any valid string that is terminated by 0)
	for(unsigned int i = 0; i < sizeof(pkt.data); ++i) {
		if(pkt.data[i] == 0)
			break;
		
		bytes.push_back(pkt.data[i]);
	}
	
	return bytes;
}

EtherPkt writeBytesToEtherPacket(char *buffer)
{
	EtherPkt etherPkt;
	
	int currentByte = 0;

	for(unsigned int i = 0; i < sizeof(MacAddr) -1 ; ++i, ++currentByte) {
		etherPkt.dst[i] = buffer[currentByte];
	}

	for(unsigned int i = 0; i < sizeof(MacAddr) -1 ; ++i, ++currentByte)
		etherPkt.src[i] = buffer[currentByte];
	
	char type[NUM_BYTES_SHORT + 1];
	for(unsigned int i = 0; i < sizeof(type); ++i)
		type[i] = 0;
	
	for(unsigned int i = 0; i < (sizeof(type) - 1); ++i, ++currentByte)
		type[i] = buffer[currentByte];

	etherPkt.type = atoi(type);
	
	char size[NUM_BYTES_SHORT + 1];
	for(unsigned int i = 0; i < sizeof(size); ++i)
		size[i] = 0;

	for(unsigned int i = 0; i < (sizeof(size) - 1); ++i, ++currentByte)
		size[i] = buffer[currentByte];

	etherPkt.size = atoi(size);

	for(int i = 0; i < ETHBUFSIZE; ++i, ++currentByte)
		etherPkt.data[i] = buffer[currentByte];
	
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
	
	for(unsigned int i = 0; i < (NUM_BYTES_SHORT - op.length()); ++i)
		bytes.push_back('0');
	
	for(unsigned int i = 0; i < op.length(); ++i)
		bytes.push_back(op[i]);
	
	// Add the source mac addr
	string srcIP = to_string((int)pkt.srcip);
	
	for(unsigned int i = 0; i < (sizeof(IPAddr) + 2 - srcIP.length()); ++i)
		bytes.push_back('0');
	
	for(unsigned int i = 0; i < srcIP.length(); ++i)
		bytes.push_back(srcIP[i]);
	
	// size - 1 because final byte is null terminator
	for(unsigned int i = 0; i < sizeof(pkt.srcmac) - 1; ++i)
		bytes.push_back(pkt.srcmac[i]);
	
	string dstIP = to_string((int)pkt.dstip);
	
	for(unsigned int i = 0; i < (sizeof(IPAddr) + 2 - dstIP.length()); ++i)
		bytes.push_back('0');
	
	for(unsigned int i = 0; i < dstIP.length(); ++i)
		bytes.push_back(dstIP[i]);
	
	// size - 1 because final byte is null terminator
	for(unsigned int i = 0; i < sizeof(pkt.dstmac) - 1; ++i)
		bytes.push_back(pkt.dstmac[i]);
	
	return bytes;
}

ARP_PKT writeBytesToArpPkt(char* buffer)
{
	ARP_PKT arpPkt;
	int currentByte = 0;
	
	char op[NUM_BYTES_SHORT + 1];
	for(unsigned int i = 0; i < sizeof(op); ++i)
		op[i] = 0;
	
	for(unsigned int i = 0; i < (sizeof(op) - 1); ++i, ++currentByte)
		op[i] = buffer[currentByte];
	
	arpPkt.op = atoi(op);
	
	char srcIp[sizeof(IPAddr) + 3];
	
	for(unsigned int i = 0; i < sizeof(srcIp); ++i)
		srcIp[i] = 0;

	for(unsigned int i = 0; i < sizeof(srcIp) -1 ; ++i)
		srcIp[i] = buffer[currentByte++];

	arpPkt.srcip = atoi(srcIp);
	
	for(unsigned int i = 0; i < sizeof(MacAddr) - 1; ++i, ++currentByte)
		arpPkt.srcmac[i] = buffer[currentByte];
	
	char dstIp[sizeof(IPAddr) + 3];
	for(unsigned int i = 0; i < sizeof(dstIp); ++i)
		dstIp[i] = 0;
	
	for(unsigned int i = 0; i < sizeof(dstIp) - 1; ++i)
		dstIp[i] = buffer[currentByte++];
	
	arpPkt.dstip = atoi(dstIp);
	
	for(unsigned int i = 0; i < sizeof(MacAddr) - 1; ++i, ++currentByte)
		arpPkt.dstmac[i] = buffer[currentByte];
	
	return arpPkt;
}

IP_PKT writeBytesToIpPkt(char *buffer)
{
	
	IP_PKT ipPkt;
	
	int currentByte = 0;
	
	char dstIp[sizeof(IPAddr) + 3];
	for(unsigned int i = 0; i < sizeof(dstIp); ++i)
		dstIp[i] = 0;
	
	for(unsigned int i = 0; i < sizeof(dstIp) - 1; ++i)
		dstIp[i] = buffer[currentByte++];
	
	ipPkt.dstip = atoi(dstIp);
	
	char srcIp[sizeof(IPAddr) + 3];
	for(unsigned int i = 0; i < sizeof(srcIp); ++i)
		srcIp[i] = 0;
	
	for(unsigned int i = 0; i < sizeof(srcIp) - 1; ++i)
		srcIp[i] = buffer[currentByte++];
	
	ipPkt.srcip = atoi(srcIp);;
	
	char length[NUM_BYTES_SHORT + 1];
	for(unsigned int i = 0; i < (sizeof(length) - 1); ++i)
		length[i] = 0;
	
	for(unsigned int i = 0; i < (sizeof(length) - 1); ++i, ++currentByte)
		length[i] = buffer[currentByte];
	
	ipPkt.length = atoi(length);
	
	for(int i = 0; i < IPBUFSIZE; ++i, ++currentByte)
		ipPkt.data[i] = buffer[currentByte];

	return ipPkt;
}
