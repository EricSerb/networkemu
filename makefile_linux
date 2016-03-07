CC     = g++
CFLAGS = -std-c++11 -Wall
INCLUDES = ether.h ip.h

%.o: %.cpp
	$(CC) -c -o $@ $< $(CXXFLAGS)

all: bridge station 

bridge: bridge.o ether.o lan.o 
	g++ -o $@ $^ $(CXXFLAGS)

station: station.o ether.o ypage.o lan.o ip.o 
	g++ -o $@ $^ $(CXXFLAGS)

clean : 
	rm -f bridge station *.o

%.o : %.c $(INCLUDES)
	$(CC) $(CFLAGS) $<

