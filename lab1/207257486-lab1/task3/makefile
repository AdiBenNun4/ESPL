all: menu

menu: menu_map.o
	gcc -m32 -g -Wall -o menu menu_map.o
	
menu_map.o: menu_map.c
	gcc -m32 -g -Wall -c -o menu_map.o menu_map.c

.PHONY: clean

clean: rm -f *.o menu