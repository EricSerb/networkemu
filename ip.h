#ifndef IP_H
#define IP_H
#include <sys/time.h>
#include <string>

/* ARP packet types */
#define ARP_REQUEST 0
#define ARP_REPLY 1

/* IP protocol types */
#define PROT_TYPE_UDP 0
#define PROT_TYPE_TCP 1
#define PROT_TYPE_OSPF 2

#define PEER_CLOSED 2

#define TYPE_IP_PKT 1
#define TYPE_ARP_PKT 0

//TODO: need to make this program work for large bufsizes. see how shorts are handled
// in ip_mac_interface toByte and fromByte functions for a description of the problem
#define BUFSIZE 100
#define ETHPKTHEADER 40
#define IPPKTHEADER 10

#define ETHBUFSIZE BUFSIZE-ETHPKTHEADER
#define IPBUFSIZE BUFSIZE-(ETHPKTHEADER+IPPKTHEADER)

typedef unsigned long IPAddr;

//XX:XX:XX:XX:XX:XX + terminating character
typedef char MacAddr[18];

/* Structure to represent an interface */

typedef struct iface {
	char ifacename[32];
	IPAddr ipaddr;
	IPAddr mask;
	MacAddr macaddr;
	char lanname[32];
} Iface;

/* Structure for a routing table entry */

typedef struct rtable {
	IPAddr destsubnet;
	IPAddr nexthop;
	IPAddr mask;
	char ifacename[32];
} Rtable;


/*queue to hold packets and tell where they need to go next
 * @buf will hold the bytes received
 * @port the port which data was received on
 * @known if the port is in the lookup table, it is known.  otherwise,
 * 	this variable is used when sending queued packets.  If known is set
 * 	to false, we must broadcast the packet to every open file descriptor
 * 	associated with the bridge (except for STDIN/OUT/ERR
 */
typedef struct packet_queue
{
	char buf[BUFSIZE];
	int socketIn;
	int socketOut;
	short arpType;
	bool known = false;
} PacketQ;

typedef struct host
{
	std::string name;
	IPAddr addr;
} Host;

#endif
