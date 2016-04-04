#include "station.h"
#include "parser.h"
#include <iostream>
#include <unistd.h>
#include <algorithm>

using namespace std;

Station::Station(bool routerFlag, string ifaceFile, string rtableFile, string hostFile)
{
	m_router = routerFlag;
	
	m_ifaces = extractInterfaces(ifaceFile);
	m_rTableEntries = extractRouteTable(rtableFile);
	m_hostMap = extractHosts(hostFile);
	
	// At construction, we have not connected to anything.  Let's make sure
	// we set a value that will not conflict with other FDs.
	m_fd = -1;
}

/**
 * Parse through commands typed by the user and perform the appropriate I/O
 */
void Station::handleUserInput(char inputBuffer[BUFSIZE])
{
	cout << "buffer: " << inputBuffer << endl;
	string line(inputBuffer);
	
	// only handle lowercase input
	transform(line.begin(), line.end(), line.begin(), ::tolower);
	
	stringstream linestream(line);
	
	string command;
	
	linestream >> command;
	
	cout << "line: " << line << endl << "command: " << command << endl;
	
	if(command == "send")
	{
		//parse pkt into a buf
		string cmd, dstHost, data, newBuf(inputBuffer);

		std::size_t i = newBuf.find(" ", 0);
		
		//found lenght of command and then copy it out. should be 4 characters
		cmd = newBuf.substr(0, i); 
		i++;

		//Now from position of i+1 should be start of host name and find end position of that
		std::size_t k = newBuf.find(" ", i);

		//copy out host name. Do k-i because that is the length we need to copy
		dstHost = newBuf.substr(i, (k - i));
		k++; //move k to start of data
		
		//get data out and now have everything extracted
		data = newBuf.substr(k, (newBuf.length() - k));

		//TODO: look up host from hosts map to get ip and then using the routing table get the MAC
		//if there is not MAC must send out an ARP and wait to send this message.
	}

	else if(command == "show")
	{
		string target;
		linestream >> target;
		
		cout << "target: " << target << endl;

		if(target == "arp")
			displayArpCache();
		else if(target == "pq")
			displayPQ();
		else if(target == "host")
			displayHostMap();
		else if(target == "iface")
			displayInterfaces();
		else if(target == "rtable")
			displayRouteTable();
		
		else //Invalid show command
		{
			cout << "Invalid show command" << endl;
			cout << " Valid options: arp, pq, host, iface, rtable" << endl << endl;
		}
	}
	else if(command == "quit")
		exit(0);
	else //catch for invalid commands
	{
		cout << "Invalid command: " << inputBuffer << endl;
	}
	//cout << "Skipped every option include invalid... Major error" << endl;
}

bool Station::router()
{
	return m_router;
}

int Station::socket()
{
	return m_fd;
}

/*
 * Determine whether or not a socket is closed.  Intended to be used for checks
 * against m_fd, where socket() is intended for assignments.
 */
bool Station::closed()
{
	return socket() == -1;
}


void Station::close()
{
	if(!closed())
		::close(m_fd);
	m_fd = -1;
}

void Station::displayArpCache()
{
	cout << "ARP CACHE" << endl;
	cout << "IP ADDRESS\tMAC ADDRESS\tTIMESTAMP" << endl;
	for(auto &it : m_arpCache) {
		cout << ntop(it.first) << "\t";
		cout << it.second.mac << "\t";
		cout << it.second.timeStamp.tv_sec;
		cout << endl;
	}
}

void Station::displayPQ()
{
	cout << "PENDING QUEUE CONTENTS" << endl;
	for(unsigned int i = 0; i < m_pendingQueue.size(); i++)
	{
		for(unsigned int j = 0; j < m_pendingQueue[i].size(); j++)
		{
			cout << m_pendingQueue[i][j] << endl;	
		}
	}
	cout << endl;
}


void Station::displayInterfaces()
{
	cout << "INTERFACES" << endl;
	cout << "NAME\tIP\tSUBNET\tMAC\tLAN" << endl;
	for(unsigned int i = 0; i < m_ifaces.size(); ++i) {
		cout << m_ifaces[i].ifacename << "\t";
		cout << ntop(m_ifaces[i].ipaddr) << "\t";
		cout << ntop(m_ifaces[i].mask) << "\t";
		
		cout << m_ifaces[i].macaddr << "\t";

		cout << m_ifaces[i].lanname;
		cout << endl;
	}
	cout << endl;
}

void Station::displayRouteTable()
{
	cout << "ROUTING TABLE" << endl;
	cout << "DESTINATION\tNEXT HOP\tMASK\tINTERFACE" << endl;
	for (unsigned int i = 0; i < m_rTableEntries.size(); ++i) {
		cout << ntop(m_rTableEntries[i].destsubnet) << "\t";
		cout << ntop(m_rTableEntries[i].nexthop) << "\t";
		cout << ntop(m_rTableEntries[i].mask) << "\t";
		cout << m_rTableEntries[i].ifacename;
		cout << endl;
	}
	cout << endl;
}

void Station::displayHostMap()
{
	cout << "HOST INFORMATION" << endl;
	cout << "HOSTNAME\tIP ADDRESS" << endl;
	for(auto &it : m_hostMap) {
		cout << it.first << "\t";
		cout << ntop(it.second) << "\t";
		cout << endl;
	}
}

/**
 * Attempt to connect to a bridge by looking at the .info file created
 * by a bridge.
 */
void Station::connectToBridge()
{
	if(m_ifaces.size() <= 0)
		return;
	
	//TODO:  support more than just one connection (try one per interface)
	string bridgeName(m_ifaces[0].lanname);
	
	string bridgeFile = bridgeName + ".info";
	
	ifstream infile(bridgeFile.c_str());
	
	if(!infile.is_open()) {
		cout << "Error opening " << bridgeFile << " ...skipping this bridge." << endl;
		return;
	}
	
	string line;
	
	getline(infile, line);
	stringstream linestream(line);
	
	string addr;
	string port;
	
	linestream >> addr >> port;
	
	struct addrinfo *res;
	
	struct addrinfo hints;
	memset(&hints, 0, sizeof hints);

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	getaddrinfo(addr.c_str(), port.c_str(), &hints, &res);
	
	m_fd = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	if(connect(m_fd, res->ai_addr, res->ai_addrlen) < 0) {
		perror("connect()");
		return;
	}
	
	cout << "Connected to " << addr << endl;
	
	// We must listen for an "accept" or "reject" string from the bridge
	int bytesRead = 0;
	char buf[BUFSIZE];
	if((bytesRead = recv(m_fd, buf, sizeof buf, 0)) <= 0) {
		cout << "recv error while listening for accept/reject" << endl;
		return;
	}

	if(strcmp(buf, "Reject") == 0) {
		cout << addr << " rejected our connection!" << endl;
		close();
		return;
	}
	else if (strcmp(buf, "Accept") == 0) {
		cout << addr << " accepted our connection!" << endl;
	}
	//TODO:  try to connect 5 times, then give up (as per writeup)
	else  {
		cout << "Not sure what " << addr << " had to say about connecting to us.  Skipping it." << endl;
		close();
		return;
	}

}
