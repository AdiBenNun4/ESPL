all: hexeditplus task4

hexeditplus: hexeditplus.o
	gcc -m32 -g -fno-pie -fno-stack-protector -Wall -o hexeditplus hexeditplus.o

task4: task4.o
	gcc -m32 -fno-pie -fno-stack-protector -g -Wall -o task4 task4.o

hexeditplus.o: task1.c
	gcc -m32 -fno-pie -fno-stack-protector -g -Wall -c -o hexeditplus.o task1.c

task4.o: task4.c
	gcc -m32 -fno-pie -fno-stack-protector -g -Wall -c -o task4.o task4.c

.PHONY: clean

clean: rm -f *.o hexeditplus                                     