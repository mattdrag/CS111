all: lab0

lab0: lab0.c
	gcc -g lab0.c -o lab0

check: lab0
	./lab0 --segfault --catch || ([ $$? -eq 3 ] && echo "Success")
	echo "test" > i.txt
	./lab0 --input=i.txt --output=o.txt
	([ $$? -eq 0 ] && echo "Success") || (echo "Failure"; exit 1)
	(diff i.txt o.txt && echo "Success") || (echo "Failure"; exit 1)

clean:
	rm -rf *.o i.txt o.txt lab0

dist:
	tar -czvf lab0-804472271.tar.gz lab0.c Makefile README segfault1.png segfault2.png
