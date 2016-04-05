#ifndef IP_LAYER_H
#define IP_LAYER_H

#include "ip.h"
#include <vector>
#include <cstring>

std::vector<unsigned char> writeArpPktToBytes(ARP_PKT pkt);


ARP_PKT writeBytesToArpPkt(char *buffer, IPAddr ip, MacAddr mac);

#endif
