#ifndef PARSER_H
#define PARSER_H

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "ip.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <map>

std::string ntop(IPAddr addr);
void dumpInterfaces(std::vector<iface> ifaces);
void dumpRtables(std::vector<rtable> entries);
void dumpHosts(std::map<std::string, IPAddr> hosts);
std::vector<iface> extractInterfaces(std::string fn);
std::vector<rtable> extractRouteTable(std::string fn);
std::map<std::string, IPAddr> extractHosts(std::string fn);

#endif