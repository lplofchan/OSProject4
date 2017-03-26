CPP=/afs/nd.edu/user14/csesoft/new/bin/g++
CPPFLAGS= -Wall -std=gnu++11 -g
LD=/afs/nd.edu/user14/csesoft/new/bin/g++
LDFLAGS= -lcurl -static-libstdc++

all: site-tester

site-tester: site-tester.o
	$(LD) $(LDFLAGS) site-tester.o -o site-tester

site-tester.o: site-tester.cpp
	$(CPP) $(CPPFLAGS) -c site-tester.cpp -o site-tester.o

clean:
	rm -f site-tester.o site-tester
