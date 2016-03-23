#ifndef MACLAYER_H
#define MACLAYER_H

#include <vector>
#include "ip.h"

std::vector<unsigned char> writeEthernetPacketToBytes(EtherPkt pkt);

#endif