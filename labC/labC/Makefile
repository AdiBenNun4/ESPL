
all: myshell Loop Pipe mypipeline test

myshell: myshell.o  LineParser.o  
	gcc -g -m32 -Wall -o myshell myshell.o LineParser.o 

test: test.o    
	gcc -g -m32 -Wall -o test test.o 

Loop: Looper.o 
	gcc -g -m32 -Wall -o Loop Looper.o

Pipe: mypipe.o 
	gcc -g -m32 -Wall -o Pipe mypipe.o

mypipeline: mypipeline.o 
	gcc -g -m32 -Wall -o mypipeline mypipeline.o

test.o: test.c
	gcc -g -m32 -Wall -c -o test.o test.c

LineParser.o: LineParser.c
	gcc -g -m32 -Wall -c -o LineParser.o LineParser.c

myshell.o: myshell.c
	gcc -g -m32 -Wall -c -o myshell.o myshell.c

Looper.o: Looper.c
	gcc -g -m32 -Wall -c -o Looper.o Looper.c

mypipe.o: mypipe.c
	gcc -g -m32 -Wall -c -o mypipe.o mypipe.c

mypipeline.o: mypipeline.c
	gcc -g -m32 -Wall -c -o mypipeline.o mypipeline.c

clean:
	rm -f *.o myshell