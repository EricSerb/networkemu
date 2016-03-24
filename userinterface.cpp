#include "userinterface.h"
#include <cstring>

using namespace std;

string parseCommands(char *buf)
{
	if(strncmp(buf, "Send", 4) == 0 || strncmp(buf, "send", 4) == 0)
	{
		//parse pkt into a buf
		string cmd, dstHost, data, newBuf(buf);

		int i = newbuf.find(" ");
		
		//found lenght of command and then copy it out. should be 4 characters
		cmd = newBuf.substr(0, i); 
		i++;

		//Now from position of i+1 should be start of host name and find end position of that
		int k = newBuf.substr(" ", i);

		//copy out host name. Do k-i because that is the length we need to copy
		dstHost = newBuf.substr(i, (k - i));
		k++; //move k to start of data
		
		//get data out and now have everything extracted
		data = newBuf.substr(k, (newBuf.length - k));

		//TODO: look up host from hosts map to get ip and then using the routing table get the MAC
		//if there is not MAC must send out an ARP and wait to send this message.
	}

	else if(strncmp(buf, "show", 4) == 0 || strncmp(buf, "Show", 4) == 0)
	{
		//show whichever file it indicates in buf
	}
	else if(strncmp(buf, "Quit", 4) == 0 || strncmp(buf, "quit", 4) == 0)
	{
		exit(0);
	}
	else //catch for invalid commands
	{
		cout << "Invalid command: " << buf << endl;
	}
}
