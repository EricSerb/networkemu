#include "ether_packet.h"
#include <cstring>
#include <iostream>

using namespace std;

EtherPkt::EtherPkt()
{
	for(unsigned int i = 0; i < sizeof(MacAddr); ++i)
		dst[i] = 0;
	strcpy(dst, "00:00:00:00:00:00");

	for(unsigned int i = 0; i < sizeof(MacAddr); ++i)
		src[i] = 0;
	strcpy(src, "00:00:00:00:00:00");
	
	type = -1;

	size = 0;
	
	for(unsigned int i = 0; i < sizeof(data); ++i)
		data[i] = 0;
}

void EtherPkt::dump()
{

	cout << "ETHER PACKET DUMP" << endl;
	cout << "dst mac: " << dst << endl;
	cout << "src mac: " << src << endl;
	cout << "type: " << type << endl;
	cout << "size: " << size << endl;
	cout << "data: " << data << endl;
}
