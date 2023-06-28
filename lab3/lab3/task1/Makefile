all: start

start: start.o
	ld -m elf_i386 start.o -o task0

start.o: start.s
	nasm -g -f elf -w+all -o start.o start.s

.PHONY: clean

clean: rm -f *.o start                                     