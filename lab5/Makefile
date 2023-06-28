all: loader

loader: start.o loader.o startup.o
	ld -o loader start.o loader.o startup.o -L/usr/lib32 -lc -T linking_script -dynamic-linker /lib32/ld-linux.so.2

start.o: start.s
	nasm -g -f elf -w+all -o start.o start.s

loader.o: loader.c
	gcc -g -m32 -Wall -c loader.c -o loader.o

startup.o: startup.s
	nasm -g -f elf -w+all -o startup.o startup.s

clean:
	rm -f *.o loader                                  