#include "userinterface.h"
#include <cstring>

using namespace std;

string parseCommands(char *buf)
{
	if(strncmp(buf, "Send", 4) == 0 || strncmp(buf, "send", 4) == 0)
	{
		//parse pkt into a buf
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
