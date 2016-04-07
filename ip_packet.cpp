#include "ip_packet.h"
#include <iostream>

using namespace std;

IP_PKT::IP_PKT()
{
	dstip = 0;
	srcip = 0;
	length = 0;
	
	for(unsigned int i = 0; i < sizeof(data); ++i)
		data[i] = 0;
}


void IP_PKT::dump()
{
	cout << "IP PACKET DUMP" << endl;
	cout << "dstip: " << dstip << endl;
	cout << "srcip: " << srcip << endl;
	cout << "length: " << length << endl;
	cout << "data: " << data << endl;
}