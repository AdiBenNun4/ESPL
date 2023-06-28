all: myELF

myELF: myELF.o
	gcc -m32 -g -fno-pie -fno-stack-protector -Wall -o myELF myELF.o

myELF.o: task1.c
	gcc -m32 -fno-pie -fno-stack-protector -g -Wall -c -o myELF.o task1.c

.PHONY: clean

clean: rm -f *.o myELF                                     