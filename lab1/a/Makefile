all: lab1a

lab1a: lab1a.c
	gcc -g -Wall -lpthread lab1a.c -o lab1a

clean:
	rm -rf *.o lab1a

dist:
	tar -czvf lab1a-804472271.tar.gz lab1a.c Makefile README
