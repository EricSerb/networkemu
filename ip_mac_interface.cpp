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
	
cout << __func__ << " " << __LINE__ << "out bytes are: " << &bytes[0] << endl;
	// Add the type.  We cannot directly add the bytes of a short to the vector,
	// so cast it to a string first
	string type = to_string((int)pkt.type);
	for(unsigned int i = 0; i < (NUM_BYTES_SHORT - type.length()); ++i)
		bytes.push_back('0');
	
	for(unsigned int i = 0; i < type.length(); ++i)
		bytes.push_back(type[i]);
cout << "string type: " << type << " and length " << type.length() << endl;
	
	// Size is also a short and must be read byte by byte.
cout << __func__ << " " << __LINE__ << "out bytes are: " << &bytes[0] << endl;
	string size = to_string((int)pkt.size);
	cout << "size string: " << size << " with length: " << size.length() << endl;

	for(unsigned int i = 0; i < (NUM_BYTES_SHORT - size.length()); ++i)
		bytes.push_back('0');
	
	for(unsigned int i = 0; i < size.length(); ++i)
		bytes.push_back(size[i]);
	
cout << __func__ << " " << __LINE__ << "out bytes are: " << &bytes[0] << endl;
	// Add the data (which is any valid string that is terminated by \0)
	for(unsigned int i = 0; i < sizeof(pkt.data); ++i) {
		if(pkt.data[i] == 0)
			break;
		
		bytes.push_back(pkt.data[i]);
	}
cout << __func__ << " " << __LINE__ << "out bytes are: " << &bytes[0] << endl;
	
	//delete typeBytes;
	//delete sizeBytes;
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
	
	cout << "size of srcIP: " << srcIP.length() << endl;
	for(unsigned int i = 0; i < srcIP.length(); ++i)
		bytes.push_back(srcIP[i]);
	
	string len = to_string((int)pkt.length);
	for(unsigned int i = 0; i < (NUM_BYTES_SHORT - len.length()); ++i)
		bytes.push_back('0');
	
	cout << __func__ << __LINE__ << "len: " << len << " pkt length: " << pkt.length << endl;
	
	for(unsigned int i = 0; i < len.length(); ++i)
		bytes.push_back(len[i]);
	
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
	
	int currentByte = 0;
	cout << __func__ << " copying buffer: " << buffer << endl;
	cout << "etherPkt.dst before copy: " << etherPkt.dst << endl;
	for(unsigned int i = 0; i < sizeof(MacAddr) -1 ; ++i, ++currentByte) {
		etherPkt.dst[i] = buffer[currentByte];
	}
	cout << "etherPkt.dst after copy: " << etherPkt.dst << endl;
	
	cout << "etherPkt.src before copy: " << etherPkt.src << endl;
	for(unsigned int i = 0; i < sizeof(MacAddr) -1 ; ++i, ++currentByte)
		etherPkt.src[i] = buffer[currentByte];
	cout << "etherPkt.src after copy: " << etherPkt.src << endl;
	
	cout << "etherPkt.type before copy: " << etherPkt.type << endl;
	
	char type[NUM_BYTES_SHORT + 1];
	for(unsigned int i = 0; i < sizeof(type); ++i)
		type[i] = 0;
	
	for(unsigned int i = 0; i < (sizeof(type) - 1); ++i, ++currentByte)
		type[i] = buffer[currentByte];

	etherPkt.type = atoi(type);
	cout << __func__ << " " << __LINE__ << " etherPkt type: " << etherPkt.type << " and actual type: " << type << endl;
	
	char size[NUM_BYTES_SHORT + 1];
	for(unsigned int i = 0; i < sizeof(size); ++i)
		size[i] = 0;

	cout << "etherPkt.type after copy: " << etherPkt.type << endl;
	
	cout << "etherPkt.size before copy: " << etherPkt.size << endl;
	for(unsigned int i = 0; i < (sizeof(size) - 1); ++i, ++currentByte)
		size[i] = buffer[currentByte];

	cout << __func__ << " " << __LINE__ << " size: " << size << endl;
	etherPkt.size = atoi(size);


	cout << "size var: " << size << "etherPkt.size after copy: " << etherPkt.size << endl;
	
	cout << "etherPkt.buf before copy: " << etherPkt.data << endl;
	for(int i = 0; i < ETHBUFSIZE; ++i, ++currentByte)
		etherPkt.data[i] = buffer[currentByte];
	cout << "etherPkt.buf after copy: " << etherPkt.data << endl;
	
	return etherPkt;
}


/**
 * Converts an ARP PKT to a byte vector, so that it can easily
 * be added to the pending queue of packets to be sent out.  Order matters here.
 * Must add bytes in the same order that we will deconstruct them in
 */
std::vector<unsigned char> writeArpPktToBytes(ARP_PKT pkt)
{
	cout << __func__ << " " << __LINE__ << endl;
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
	cout << __func__ << " " << __LINE__ << " bytes: " << &bytes[0] << endl;
	
	for(unsigned int i = 0; i < (sizeof(IPAddr) + 2 - srcIP.length()); ++i)
		bytes.push_back('0');
	
	cout << __func__ << " " << __LINE__ << " bytes: " << &bytes[0] << endl;
	
	cout << "size of srcIP: " << srcIP.length() << endl;
	for(unsigned int i = 0; i < srcIP.length(); ++i)
		bytes.push_back(srcIP[i]);
	
	cout << __func__ << " " << __LINE__ << " bytes: " << &bytes[0] << endl;
	
	// size - 1 because final byte is null terminator.
	// This will have to be added in at the other side
	for(unsigned int i = 0; i < sizeof(pkt.srcmac) - 1; ++i)
		bytes.push_back(pkt.srcmac[i]);
	
	string dstIP = to_string((int)pkt.dstip);
	cout << "dstIp: " << dstIP << " pkt dst ip: " << pkt.dstip << " ntop: " << ntop(pkt.dstip) << endl;
	
	cout << __func__ << " " << __LINE__ << " bytes: " << &bytes[0] << endl;
	for(unsigned int i = 0; i < (sizeof(IPAddr) + 2 - dstIP.length()); ++i)
		bytes.push_back('0');
	
	cout << __func__ << " " << __LINE__ << " bytes: " << &bytes[0] << endl;
	
	for(unsigned int i = 0; i < dstIP.length(); ++i)
		bytes.push_back(dstIP[i]);
	cout << __func__ << " " << __LINE__ << " bytes: " << &bytes[0] << endl;
	
	// size - 1 because final byte is null terminator.
	// This will have to be added in at the other side
	cout << "dstMac: " << pkt.dstmac << endl;
	for(unsigned int i = 0; i < sizeof(pkt.dstmac) - 1; ++i)
		bytes.push_back(pkt.dstmac[i]);
	
	return bytes;
}

ARP_PKT writeBytesToArpPkt(char* buffer)
{
	cout << __func__ << " " << __LINE__ << endl;
	cout << "buffer: " << buffer << endl;
	ARP_PKT arpPkt;
	int currentByte = 0;
	
	char op[NUM_BYTES_SHORT + 1];
	for(unsigned int i = 0; i < sizeof(op); ++i)
		op[i] = 0;
	
	for(unsigned int i = 0; i < (sizeof(op) - 1); ++i, ++currentByte)
		op[i] = buffer[currentByte];
	
	arpPkt.op = atoi(op);
	
	char srcIp[sizeof(IPAddr) + 3];
	cout << "sizeof srcIP: " << sizeof(IPAddr) + 3 << endl;
	
	for(unsigned int i = 0; i < sizeof(srcIp); ++i)
		srcIp[i] = 0;
	cout << "currentByte : " << currentByte << endl;
	for(unsigned int i = 0; i < sizeof(srcIp) -1 ; ++i)
		srcIp[i] = buffer[currentByte++];
	cout << "currentByte : " << currentByte << endl;
	cout << "srcIP: " << srcIp << endl;
	arpPkt.srcip = atoi(srcIp);
	
	cout << "src ip: " << arpPkt.srcip << " and ntop(): " << ntop(arpPkt.srcip) << endl;
	cout << "buffer now: " << &buffer[currentByte] << endl;
	
	for(unsigned int i = 0; i < sizeof(MacAddr) - 1; ++i, ++currentByte)
		arpPkt.srcmac[i] = buffer[currentByte];
	
	cout << "src mac: " << arpPkt.srcmac << endl;
	cout << "buffer now: " << &buffer[currentByte] << endl;
	
	char dstIp[sizeof(IPAddr) + 3];
	for(unsigned int i = 0; i < sizeof(dstIp); ++i)
		dstIp[i] = 0;
	
	for(unsigned int i = 0; i < sizeof(dstIp) - 1; ++i)
		dstIp[i] = buffer[currentByte++];
	
	cout << "dstIP: " << dstIp << endl;
	arpPkt.dstip = atoi(dstIp);
	
	cout << "dst ip: " << arpPkt.dstip << " and ntop(): " << ntop(arpPkt.dstip) << endl;
	cout << "buffer now: " << &buffer[currentByte] << endl;
	
	for(unsigned int i = 0; i < sizeof(MacAddr) - 1; ++i, ++currentByte)
		arpPkt.dstmac[i] = buffer[currentByte];
	
	cout << "dst mac: " << arpPkt.dstmac << endl;
	cout << "buffer now: " << &buffer[currentByte] << endl;
	
	return arpPkt;
}

IP_PKT writeBytesToIpPkt(char *buffer)
{
	
	IP_PKT ipPkt;
	cout << __func__ << " " << __LINE__ << endl;
	cout << "buffer: " << buffer << endl;
	
	int currentByte = 0;
	
	char dstIp[sizeof(IPAddr) + 3];
	for(unsigned int i = 0; i < sizeof(dstIp); ++i)
		dstIp[i] = 0;
	
	for(unsigned int i = 0; i < sizeof(dstIp) - 1; ++i)
		dstIp[i] = buffer[currentByte++];
	
	cout << "dstIP: " << dstIp << endl;
	ipPkt.dstip = atoi(dstIp);
	cout << "ipPkt dst after copy: " << ipPkt.dstip << " ntop: " << ntop(ipPkt.dstip) << endl;
	cout << "buffer now: " << &buffer[currentByte] << endl;
	
	char srcIp[sizeof(IPAddr) + 3];
	for(unsigned int i = 0; i < sizeof(srcIp); ++i)
		srcIp[i] = 0;
	
	for(unsigned int i = 0; i < sizeof(srcIp) - 1; ++i)
		srcIp[i] = buffer[currentByte++];
	
	cout << "srcIP: " << srcIp << endl;
	ipPkt.srcip = atoi(srcIp);
	
	cout << "src ip: " << ipPkt.srcip << " and ntop(): " << ntop(ipPkt.srcip) << endl;
	cout << "buffer now: " << &buffer[currentByte] << endl;
	
	char length[NUM_BYTES_SHORT + 1];
	for(unsigned int i = 0; i < (sizeof(length) - 1); ++i)
		length[i] = 0;
	
	for(unsigned int i = 0; i < (sizeof(length) - 1); ++i, ++currentByte)
		length[i] = buffer[currentByte];

	cout << __func__ << " " << __LINE__ << " length: " << length<< endl;
	ipPkt.length = atoi(length);
	
	cout << "ip length: " << ipPkt.length << " and size of IP pkt: " << sizeof(IP_PKT) << endl;
	cout << "buffer now: " << &buffer[currentByte] << endl;
	
	for(int i = 0; i < IPBUFSIZE; ++i, ++currentByte)
		ipPkt.data[i] = buffer[currentByte];
	
	
	cout << "ipPkt.buf after copy: " << ipPkt.data << endl;
	
	return ipPkt;
}



	
