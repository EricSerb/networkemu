#ifndef MAC_LAYER_H
#define MAC_LAYER_H

#include <vector>
#include "ip.h"

std::vector<unsigned char> writeEthernetPacketToBytes(EtherPkt pkt);

#endif