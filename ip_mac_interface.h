#ifndef IP_MAC_INTERFACE_H
#define IP_MAC_INTERFACE_H

#include "ip.h"
#include <vector>
#include <cstring>

// DATA LINK LAYER
std::vector<unsigned char> writeEthernetPacketToBytes(EtherPkt pkt);

// NETWORK LAYER
std::vector<unsigned char> writeArpPktToBytes(ARP_PKT pkt);
EtherPkt writeBytesToEtherPacket(char *buffer);
ARP_PKT writeBytesToArpPkt(char *buffer);
IP_PKT writeBytesToIpPkt(char *buffer);

#endif
