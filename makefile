CC     = g++
CXXFLAGS = -std=c++11 -Wall -I.
INCLUDES = ip.h

%.o: %.cpp
	$(CC) -c -o $@ $< $(CXXFLAGS)

all: bridge station 

bridge: bridge.o parser.o
	g++ -o $@ $^ $(CXXFLAGS)

station: station_main.o station.o parser.o maclayer.o ip_layer.o
	g++ -o $@ $^ $(CXXFLAGS)

clean : 
	rm -f bridge station *.o
