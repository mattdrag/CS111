all: client.c server.c

server: server.c
	gcc -lpthread -lmcrypt server.c -o server

client: client.c
	gcc -lpthread -lmcrypt client.c -o client

clean:
	rm -rf *.o client server

dist:
	tar -czvf lab1b-804472271.tar.gz client.c server.c Makefile README
