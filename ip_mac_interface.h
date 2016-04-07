#ifndef IP_MAC_INTERFACE_H
#define IP_MAC_INTERFACE_H

#include "ip.h"
#include <vector>
#include <cstring>
#include "ether_packet.h"
#include "arp_packet.h"
#include "ip_packet.h"

// DATA LINK LAYER
std::vector<unsigned char> writeEthernetPacketToBytes(EtherPkt pkt);
EtherPkt writeBytesToEtherPacket(char *buffer);

// NETWORK LAYER
std::vector<unsigned char> writeArpPktToBytes(ARP_PKT pkt);
ARP_PKT writeBytesToArpPkt(char *buffer);
IP_PKT writeBytesToIpPkt(char *buffer);
std::vector< unsigned char > writeIpPktToBytes(IP_PKT pkt);

#endif
