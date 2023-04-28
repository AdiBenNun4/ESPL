
all: start Loop Pipe

start: myshell.o  LineParser.o  
	gcc -g -m32 -Wall -o start myshell.o LineParser.o 

Loop: Looper.o 
	gcc -g -m32 -Wall -o Loop Looper.o

Pipe: mypipe.o 
	gcc -g -m32 -Wall -o Pipe mypipe.o

LineParser.o: LineParser.c
	gcc -g -m32 -Wall -c -o LineParser.o LineParser.c

myshell.o: myshell.c
	gcc -g -m32 -Wall -c -o myshell.o myshell.c

Looper.o: Looper.c
	gcc -g -m32 -Wall -c -o Looper.o Looper.c

mypipe.o: mypipe.c
	gcc -g -m32 -Wall -c -o mypipe.o mypipe.c

clean:
	rm -f *.o start