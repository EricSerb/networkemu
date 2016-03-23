CC     = g++
CXXFLAGS = -std=c++11 -Wall
DEPS = ether.h ip.h

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CXXFLAGS)

all: bridge station 

bridge: bridge.o
	g++ -o $@ $^ $(CXXFLAGS)

station: station.o parser.o
	g++ -o $@ $^ $(CXXFLAGS)

clean : 
	rm -f bridge station *.o
