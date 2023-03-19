name = hyperoute
flags = -Wall -Wextra -Werror -std=c17

all: socket.o helpers.o
	cc -o $(name) socket.o helpers.o $(flags) main.c

socket:
	cc -o socket.o -c $(flags) socket.c

helpers:
	cc -o helpers.o -c $(flags) helpers.c

clean:
	rm -f *.o

distclean:
	rm -f *.o $(name)

install:
	cp $(name) /usr/local/bin

uninstall:
	rm -f /usr/local/bin/$(name)