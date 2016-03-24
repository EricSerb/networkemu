CC     = g++
CXXFLAGS = -std=c++11 -Wall -I.
INCLUDES = ip.h

%.o: %.cpp
	$(CC) -c -o $@ $< $(CXXFLAGS)

all: bridge station 

bridge: bridge.o parser.o
	g++ -o $@ $^ $(CXXFLAGS)

station: station.o parser.o maclayer.o userinterface.o
	g++ -o $@ $^ $(CXXFLAGS)

clean : 
	rm -f bridge station *.o
