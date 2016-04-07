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

#define BUFSIZE 1024
#define ETHPKTHEADER 40
#define IPPKTHEADER 10

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

/* mapping between interface name and socket id representing link */
typedef struct itface2link {
	char ifacename[32];
	int sockfd;
} ITF2LINK;

/* Structure for a routing table entry */

typedef struct rtable {
	IPAddr destsubnet;
	IPAddr nexthop;
	IPAddr mask;
	char ifacename[32];
} Rtable;


/* Structure for an ARP cache entry */

typedef struct arpcache {
	IPAddr ipaddr;
	MacAddr macaddr;
} Arpc;

/*--------------------------------------------------------------------*/

/* The Structure of the IP datagram and ARP packets go here */

/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/* Structure for ARP packets */

/*list of arp cache, to use this one to maintain current cache*/

typedef struct arp_list {
	Arpc *arp_item;
	struct arp_list *next;
} ARP_LIST;

/*IP packet format*/
typedef struct ip_pkt
{
	IPAddr  dstip;
	IPAddr  srcip;
	short   length;
	char    data[BUFSIZE-(ETHPKTHEADER+IPPKTHEADER)];
} IP_PKT;

/*queue for ip packet that has not yet sent out
typedef struct p_queue
{
	IPAddr next_hop_ipaddr;
	IPAddr dst_ipaddr;
	char *pending_pkt;
	struct p_queue *next;
} PENDING_QUEUE;*/

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
	short type;
	bool known = false;
} PacketQ;

/*-------------------------------------------------------------------- */

#define MAXHOSTS 32
#define MAXINTER 32

typedef struct host
{
	std::string name;
	IPAddr addr;
} Host;

typedef struct lan_rout {
	short router_attached;
	short counter;
} LAN_ROUT;

/*
//some global variables here
Host host[MAXHOSTS];
int hostcnt;

Iface iface_list[MAXINTER];
//if there is router on this lan, 1; else 0
LAN_ROUT lan_router[MAXINTER];
ITF2LINK link_socket[MAXINTER];
int intr_cnt; //counter for interface 

Rtable rt_table[MAXHOSTS*MAXINTER];
int rt_cnt;

//PENDING_QUEUE *pending_queue;
ARP_LIST *arp_cache;

int ROUTER;
*/
#endif
