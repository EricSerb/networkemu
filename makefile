CC     = g++
CXXFLAGS = -std=c++11 -Wall -I.
INCLUDES = ip.h

%.o: %.cpp
	$(CC) -c -o $@ $< $(CXXFLAGS)

all: bridge station 

bridge: bridge.o parser.o ether_packet.o
	g++ -o $@ $^ $(CXXFLAGS)

station: station_main.o station.o parser.o ether_packet.o ip_mac_interface.o
	g++ -o $@ $^ $(CXXFLAGS)

clean : 
	rm -f bridge station *.o
