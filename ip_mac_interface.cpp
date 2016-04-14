#include "ip_mac_interface.h"
#include <iostream>
#include <cstring>
#include "parser.h"

using namespace std;

signed short ByteToShort(unsigned char* bytes){

    signed short result = 0;
    result = (result<<8) + bytes[1]; // heigh byte
    result = (result<<8) + bytes[0]; // low byte
    return result;
}

void ShortToByte(signed short num, unsigned char* bytes){

    bytes[1] = num & 0xFF00; // heigh byte
    bytes[0] = num & 0x00FF; // low byte
}

short toShort(const char byte) {
	signed short result = 0;
	result = (result<<8) + byte; // low byte
	return result;
}

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
	cout << "string type: " << type << " and length " << type.length() << endl;
	
	for(unsigned int i = 0; i < type.length(); ++i)
		bytes.push_back(type[i]);
	/*unsigned char* typeBytes = new unsigned char[3];
	cout << "pkt type: " << pkt.type << endl;
	ShortToByte(pkt.type, typeBytes);
	short res = ByteToShort(typeBytes);
	
	//short res = toShort(b);
	cout << "the res is: " << res << endl;
	
	for(unsigned int i = 0; i < sizeof(typeBytes) - 1; ++i)
		bytes.push_back(typeBytes[i]);
	
	unsigned char* sizeBytes = new unsigned char[3];
	cout << "pkt size: " << pkt.size << endl;
	ShortToByte(pkt.size, sizeBytes);
	short resu = ByteToShort(sizeBytes);
	
	//short res = toShort(b);
	cout << "the resu is: " << resu << endl;
	
	for(unsigned int i = 0; i < sizeof(sizeBytes) - 1; ++i)
		bytes.push_back(sizeBytes[i]);
	*/
	
	// Size is also a short and must be read byte by byte.
cout << __func__ << " " << __LINE__ << "out bytes are: " << &bytes[0] << endl;
	string size = to_string((int)pkt.size);
	cout << "size string: " << size << " with length: " << size.length() << endl;

	// TODO: pkt.size represents how full a data buffer is.  This could be
	// 1 - 4 digits long (max of 1024), so for all size strings less than 4 characters,
	// maybe we should pad it up to 4 characters (precede the size with 0's).  That way,
	// we can just extract 4 characters on the receivers end.  This lets us know where the 'size'
	// field ends and where the data buffer begins
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
	
	string dst = to_string((int)pkt.dstip);
	for(unsigned int i = 0; i < dst.length(); ++i)
		bytes.push_back(dst[i]);
	
	string src = to_string((int)pkt.srcip);
	for(unsigned int i = 0; i < src.length(); ++i)
		bytes.push_back(src[i]);
	
	string len = to_string((int)pkt.length);
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
	
	/* memcpy(etherPkt.dst, &buffer[0], sizeof(MacAddr));
	cout << __func__ << " dst: " << etherPkt.dst << endl;
	memcpy(etherPkt.src, &buffer[18], 18);
	memcpy(&etherPkt.type, &buffer[36], 2);
	memcpy(&etherPkt.size, &buffer[38], 2);
	memcpy(&etherPkt.type, &buffer[36], 2);
	memcpy(&etherPkt.size, &buffer[38], 2);
	memcpy(etherPkt.data, &buffer[40], BUFSIZE-ETHPKTHEADER); */
	
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
	
	unsigned char type = buffer[currentByte++];
	if(type == '0')
		etherPkt.type = 0;
	else if(type == '1')
		etherPkt.type = 1;
	else {
		cout << __func__ << " " << __LINE__ << " ERROR" << endl;
		exit(1);
	}
	char size[3];

	cout << "etherPkt.type after copy: " << etherPkt.type << endl;
	
	cout << "etherPkt.size before copy: " << etherPkt.size << endl;
	//memcpy(&etherPkt.size, (short*)&buffer[currentByte], 1);
	for(unsigned int i = 0; i < sizeof(size) - 1; ++i, ++currentByte)
		size[i] = buffer[currentByte];
	size[2] = 0;
	cout << __func__ << " " << __LINE__ << " size: " << size << endl;
	etherPkt.size = atoi(size);


	//cout << "arp packet size: " << sizeof(ARP_PKT) << " etherPkt.size after copy: " << etherPkt.size << endl;
	
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
	
	for(unsigned int i = 0; i < op.length(); ++i)
		bytes.push_back(op[i]);
	
	// Add the source mac addr
	string srcIP = to_string((int)pkt.srcip);
	
	cout << "size of srcIP: " << srcIP.length() << endl;
	for(unsigned int i = 0; i < srcIP.length(); ++i)
		bytes.push_back(srcIP[i]);
	
	// size - 1 because final byte is null terminator.
	// This will have to be added in at the other side
	for(unsigned int i = 0; i < sizeof(pkt.srcmac) - 1; ++i)
		bytes.push_back(pkt.srcmac[i]);
	
	string dstIP = to_string((int)pkt.dstip);
	cout << "dstIp: " << dstIP << " pkt dst ip: " << pkt.dstip << " ntop: " << ntop(pkt.dstip) << endl;
	for(unsigned int i = 0; i < dstIP.length(); ++i)
		bytes.push_back(dstIP[i]);
	
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
	
	unsigned char op = buffer[currentByte++];
	if(op == '0')
		arpPkt.op = 0;
	else if(op == '1')
		arpPkt.op = 1;
	else {
		cout << __func__ << " " << __LINE__ << " ERROR" << endl;
		exit(1);
	}
	
	char srcIp[sizeof(IPAddr) + 1];
	cout << "sizeof srcIP: " << sizeof(IPAddr) + 1 << endl;
	
	for(unsigned int i = 0; i < sizeof(srcIp); ++i)
		srcIp[i] = 0;
	
	for(unsigned int i = 0; i < sizeof(srcIp); ++i)
		srcIp[i] = buffer[currentByte++];
	
	cout << "srcIP: " << srcIp << endl;
	arpPkt.srcip = atoi(srcIp);
	
	cout << "src ip: " << arpPkt.srcip << " and ntop(): " << ntop(arpPkt.srcip) << endl;
	cout << "buffer now: " << &buffer[currentByte] << endl;
	
	for(unsigned int i = 0; i < sizeof(MacAddr) - 1; ++i, ++currentByte)
		arpPkt.srcmac[i] = buffer[currentByte];
	
	cout << "src mac: " << arpPkt.srcmac << endl;
	cout << "buffer now: " << &buffer[currentByte] << endl;
	
	char dstIp[sizeof(IPAddr) + 1];
	for(unsigned int i = 0; i < sizeof(dstIp); ++i)
		dstIp[i] = 0;
	
	for(unsigned int i = 0; i < sizeof(dstIp); ++i)
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
	
	char dstIp[sizeof(IPAddr) + 1];
	for(unsigned int i = 0; i < sizeof(dstIp); ++i)
		dstIp[i] = 0;
	
	for(unsigned int i = 0; i < sizeof(dstIp); ++i)
		dstIp[i] = buffer[currentByte++];
	
	cout << "dstIP: " << dstIp << endl;
	ipPkt.dstip = atoi(dstIp);
	cout << "ipPkt dst after copy: " << ipPkt.dstip << " ntop: " << ntop(ipPkt.dstip) << endl;
	cout << "buffer now: " << &buffer[currentByte] << endl;
	
	char srcIp[sizeof(IPAddr) + 1];
	for(unsigned int i = 0; i < sizeof(srcIp); ++i)
		srcIp[i] = 0;
	
	for(unsigned int i = 0; i < sizeof(srcIp); ++i)
		srcIp[i] = buffer[currentByte++];
	
	cout << "srcIP: " << srcIp << endl;
	ipPkt.srcip = atoi(srcIp);
	
	cout << "src ip: " << ipPkt.srcip << " and ntop(): " << ntop(ipPkt.srcip) << endl;
	cout << "buffer now: " << &buffer[currentByte] << endl;
	
	char length[3];
	
	for(unsigned int i = 0; i < sizeof(length) - 1; ++i, ++currentByte)
		length[i] = buffer[currentByte];
	length[2] = 0;
	cout << __func__ << " " << __LINE__ << " length: " << length<< endl;
	ipPkt.length = atoi(length);
	
	cout << "ip length: " << ipPkt.length << " and size of IP pkt: " << sizeof(IP_PKT) << endl;
	cout << "buffer now: " << &buffer[currentByte] << endl;
	
	for(int i = 0; i < IPBUFSIZE; ++i, ++currentByte)
		ipPkt.data[i] = buffer[currentByte];
	cout << "ipPkt.buf after copy: " << ipPkt.data << endl;
	
	return ipPkt;
	/*
	
	int i = 0, j = 0;
	
	//memcpy areas of buffer to form the ip packet that was just sent to us
	//memcpy(&(pkt.dstip), &(buffer[0]), 4);
	
	//memcpy(&(pkt.srcip), &(buffer[4]), 4);
	
	//memcpy(&(pkt.length), &(buffer[14]), 2);
	
	//memcpy((pkt.data), &(buffer[16]), pkt.length);
	
	IP_PKT pkt;
	char ip[4];
	for(j = 0; j < 4; i++, j++)
		ip[j] = buffer[i];
	pkt.dstip = atoi(ip);
	
	for(j = 0; j < 4; i++, j++)
		ip[j] = buffer[i];
	pkt.srcip = atoi(ip);
	
	char length[2];
	for(j = 0; j < 2; i++, j++)
		length[j] = buffer[i];
	pkt.length = atoi(length);
	
	for(int i = 16, j = 0; i < pkt.length; i++, j++)
		pkt.data[j] = buffer[i];
	
	
	return pkt;*/
}



	
