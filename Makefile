all: socket.o helpers.o
	cc -o hyperoute socket.o helpers.o main.c

socket:
	cc -o socket.o -c socket.c

helpers:
	cc -o helpers.o -c helpers.c

clean:
	rm -f *.o hyperoute

install:
	cp hyperoute /usr/local/bin

uninstall:
	rm -f /usr/local/bin/hyperoute