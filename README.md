****************************************************
* Developed by Brandon Stephens and Eric Serbousek *
*                                                  *
* Data/Computer Communications (CNT5505 - 01)      *
* Computer Science, FSU April 14th 2016            *
****************************************************

1) Brandon Stephens
	-Email: bms11e@my.fsu.edu
   Eric Serbousek
   	-Email: els16@my.fsu.edu

2) How to compile the code
	-In the folder type the command "make". This will compile all the code and create the executable files.

3) Commands supported in stations/routers/bridges
	3.1 Stations:
		send <destination> <message>	// send a message to a destination host
		show arp			// show the ARP cache table information
		show pq				// how the pending_queue
		show host			// show the IP/name mapping table
		show iface			// show the interface information
		show rtable			// show the contents of routing table
		quit				// close the station

	3.2 routers:
		show arp			// show the ARP cache table information
		show pq				// how the pending_queue
		show host			// show the IP/name mapping table
		show iface			// show the interface information
		show rtable			// show the contents of routing table
		quit				// close the station

	3.3 bridges:
		show sl 			// show the contents of the self-learning table
		quit				// close the bridge

4) To start and run the emulation:
	-You will need to open up a terminal window for each of the stations, routers, and bridges. 
	-Bridges must be started first and they can be started by "bridge lan-name #OfPorts"
	-Routers can then be started by using the commands "station -router ifaces/ifaces.r1 rtables/rtable.r1   hosts" 
	-Stations can be started only after bridges and routers by using the commands "station -no ifaces/ifaces.a rtables/rtable.a 
	hosts"
	
	Our topology and naming scheme

	   -------A---------
	   |               |
	  cs1-----r1------cs2-------r2-------cs3
	   |               | 		      |
	   B               C                  D

	cs1, cs2, and cs3 are bridges
	r1 and r2 are routers
	A-D are hosts/stations
	Note that A is multi-homed, but it is not a router

	This is how we connect everything and our code is working for this topology. The naming is also in our files in this way.
	The above topology is created by issuing each of these commands in separate terminals IN ORDER:
	
	./bridge cs1 10
	./bridge cs2 10
	./bridge cs3 10
	./station -router ifaces/ifaces.r1 rtables/rtable.r1 hosts
	./station -router ifaces/ifaces.r2 rtables/rtable.r2 hosts
	./station -no ifaces/ifaces.a rtables/rtable.a hosts
	./station -no ifaces/ifaces.b rtables/rtable.b hosts
	./station -no ifaces/ifaces.c rtables/rtable.c hosts
	./station -no ifaces/ifaces.d rtables/rtable.d hosts

5) Difficulities that we have encountered during the development of the project:
	-One of the major problems we ran into was changing the structures to an array of bytes (unsigned chars).
		We tried using the function memcpy() at first to just copy the memory from the structure to the array.
		We found that this often did not work as expected and was not copying the bytes correctly.
		This could have been due to use passing the wrong type of pointers to the function.
		We ended up using loops to copy the bytes into a vector as a way around using memcpy(). Using this method fixed our problem.

	-Another problem we ran into is with the size field in the different packet types.
		Since we were not using memcpy, but we were now looping and copying structures byte for byte,
		we had problems with how many times to loop since the size variable could be anywhere from 2-4 digits long.
		The way we decided to fix this was to pad the size field by preappending 0's in order to always make that field contain 4 digits.

	-We had the same problem as above with size but with IPAddr.
		We assumed that all of the IP addresses when in the unsigned long form would be 9 digits long since they were for all of the stations and routers, until we
		started using station D. Station D's IP was actually 10 digits long. So we did the same fix that we had with size by padding the
		IPAddr field with preappended 0's. This fixed our problem.

	-One other problem we had with the routers was that on broadcast packets they would always send the
		packet out to bridge that the IP determined it should go out to. The problm we had was there would be a constant loop with broadcasts,
		since the bridge would broadcast the packet and the routers connected to it would receive the packet and see it needed to go back to that
		bridge and would then send it back. The Bridge would then see the packet would need to be broadcast again and send it back to the routers.
		We fixed this by stopping the routers  from sending back to the port it received on.
