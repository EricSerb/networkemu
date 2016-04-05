#include "ip_layer.h"

using namespace std;


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


	
