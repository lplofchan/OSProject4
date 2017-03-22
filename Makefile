all: site-tester

site-tester: site-tester.o
	g++ -std=gnu++11 -g -lcurl site-tester.o -o site-tester

site-tester.o: site-tester.cpp
	g++ -std=gnu++11 -g -Wall -c -lcurl site-tester.cpp -o site-tester.o

clean:
	rm -f site-tester.o site-tester