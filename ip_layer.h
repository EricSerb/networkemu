#ifndef IP_LAYER_H
#define IP_LAYER_H

#include "ip.h"
#include <vector>

std::vector<unsigned char> writeArpPktToBytes(ARP_PKT pkt);

#endif