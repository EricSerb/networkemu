/*----------------------------------------------------------------*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>

//Eric's headers used in server.cpp
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <unistd.h>

//New headers
#include <queue>
/*----------------------------------------------------------------*/

using namespace std;

/* bridge : recvs pkts and relays them */
/* usage: bridge lan-name max-port */
int 
main (int argc, char *argv[])
{	
	/* create the symbolic links to its address and port number
	* so that others (stations/routers) can connect to it
	*/

	/* listen to the socket.
	* two cases:
	* 1. connection open/close request from stations/routers
	* 2. regular data packets
	*/


	//max fd #, listening socket descriptor, newly accepted fd, # bytes in buff
	int fdmax, listener, newfd, nbytes;
	//buff for client interaction
	char buf[1024];
	int yes = 1;
	int addrlen;
	int i, j;
	int numPorts = atoi(argv[2]);
	string message;
	//server address
	struct sockaddr_in saddr;
	//client addr
	struct sockaddr_in caddr;
	//master fd list
	fd_set master;
	//temp fd list for select()
	fd_set read_fds;
	char linprog2[] = {"128.186.120.34"};

	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	memset(&saddr, 0, sizeof(saddr));
	memset(&caddr, 0, sizeof(caddr));

	//socket
	if((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		cout << "Server socket() error... CRASH!" << endl;
		exit(1);
	}

	cout << "Server socket() ok..." << endl;

	//addr already in use
	if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		cout << "Server setsockopt() error... CRASH!" << endl;
		exit(1);
	}

	cout << "Server setsockopt ok..." << endl;

	char temp[255];
	gethostname(temp, 255);
	struct hostent *host_entry;
	host_entry = gethostbyname(temp);
	memcpy(&saddr.sin_addr.s_addr, host_entry->h_addr, host_entry->h_length);

	//bind
	saddr.sin_family = AF_INET;
	//inet_aton(linprog2, &saddr.sin_addr);
	//memset(&(saddr.sin_zero), 0, 8);

	if(bind(listener, (struct sockaddr *)&saddr, sizeof(saddr)) == -1)
	{
		cout << "Server bind() error... CRASH!" << endl;
		exit(1);
	}
	//unsigned long p = saddr.sin_port;

	//cout << ntohl(p) << endl;

	
	//saddr.sin_port = p;
	cout << "Server bind() ok..." << endl;
	
	//cout << "Server address = " << saddr.sin_addr.s_addr << endl;
	//cout << "Server port number = " << saddr.sin_port << endl;

	if(listen(listener, 10) == -1)
	{
		cout << "Server listen() error... CRASH!" << endl;
		exit(1);
	}
	
	cout << "Server listen() ok..." << endl;
	
	socklen_t len = sizeof(saddr);
	if(getsockname(listener, (struct sockaddr*)&saddr, &len) == -1)
	{
		cout << "Server getsockname() error... CRASH!" << endl;
		exit(1);
	}
	//add listener to the master set
	FD_SET(listener, &master);
	
	//keeping track of biggest fd
	fdmax = listener; 

	cout << endl;
	cout << "Server name = " << temp << endl;
	cout << "Server address = " << inet_ntoa(saddr.sin_addr) << endl;
	cout << "Server port number = " << ntohs(saddr.sin_port) << endl;

	for(;;)
	{
		memset(&buf, 0, sizeof(buf));
		read_fds = master;

		if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
		{
			cout << "Server select() error... CRASH!" << endl;
			exit(1);
		}
		//cout << "Server select() ok..." << endl;

		//check existing connections for anything to read
		for(i = 0; i <= fdmax; i++)
		{
			if(FD_ISSET(i, &read_fds))
			{
				if(i == listener)
				{
					//handle new connections
					addrlen = sizeof(caddr);
					if((newfd = accept(listener, (struct sockaddr *)&caddr, (socklen_t *) &addrlen)) == -1)
					{
						cout << "Server accpet() error...." << endl;
					}
					else
					{
					
						FD_SET(newfd, &master); //adding to master set
						if(newfd > fdmax)
						{
							fdmax = newfd;
						}

						cout << endl << endl << argv[0] << ": New connection from " << inet_ntoa(caddr.sin_addr) << " on socket " << newfd << endl;

						cout << endl;
						cout << "Server name = " << temp << endl;
						cout << "Server address = " << inet_ntoa(saddr.sin_addr) << endl;
						cout << "Server port number = " << ntohs(saddr.sin_port) << endl;
					}
				}
				else
				{
					//handle data from client
					if((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0)
					{
						//error or connection closed
						if(nbytes == 0)
						{
							cout << argv[0] << ": socket " << i << " hung up" << endl;
						}
						else
						{
							cout << "Server recv() error..." << endl;
						}

						close(i);
						FD_CLR(i, &master);

					}
					else
					{
						//we got some data from the client
						for(j = 0; j <= fdmax; j++)
						{
							//send to everyone
							if(FD_ISSET(j, &master))
							{
								//except the listener and server
								if(j != listener && j != i)
								{
									//cout << buf << " i = " << j << endl;
									if(send(j, buf, nbytes, 0) == -1)
									{
										cout << "Server send() error..." << endl;
									}
								}
							}
						}
					}
				}
			}
		}
	}
}
/*----------------------------------------------------------------*/
