#include "userinterface.h"
#include <cstring>
#include <map>

/*
using namespace std;

typedef map<string, printFunctions> M;

void printARP();
void printPQ();
void printHOST();
void printIFACE();
void printRTABLE();

M printMap;

printMap.insert(make_pair("arp", &printARP));
printMap.insert(make_pair("pq", &printPQ));
printMap.insert(make_pair("host", &printHOST));
printMap.insert(make_pair("iface", &printIFACE));
printMap.insert(make_pair("rtable", &printRTABLE));

string parseCommands(char *buf)
{
	if(strncmp(buf, "Send", 4) == 0 || strncmp(buf, "send", 4) == 0)
	{
		//parse pkt into a buf
		string cmd, dstHost, data, newBuf(buf);

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

	else if(strncmp(buf, "show", 4) == 0 || strncmp(buf, "Show", 4) == 0)
	{
		//show whichever file it indicates in buf
		string newBuf(buf);
		size_t i = newBuf.find(" ", 0);
		i++;
		//just need to get the command out of the buf and that is it
		string cmd = newBuf.substr(i, (newBuf.length() - i));

		auto& it = printMap.find(cmd);

		if(it == printMap.end())
		{
			cout << "Invalid show command." << endl;

			return "Invalid";
		}
		it.second();
	}
	else if(strncmp(buf, "Quit", 4) == 0 || strncmp(buf, "quit", 4) == 0)
	{
		exit(0);
	}
	else //catch for invalid commands
	{
		cout << "Invalid command: " << buf << endl;
		return "Invalid";
	}
	return "Catch... should never reach this";
}

void printARP()
{

}

void printPQ()
{

}

void printHOST()
{

}

void printIFACE()
{

}

void printRTABLE()
{

}
*/