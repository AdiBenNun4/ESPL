#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
	char *str="Adi and Aviv";
    char buffer [12];
    int mypipe[2];
    if(pipe(mypipe) < 0) _exit(1);

    if(fork() == 0){ //child code
        write (mypipe[1] , str , 12);
    }
    else{ //parent code
        read(mypipe[0] , buffer , 12 );
        printf("%s\n",buffer);
    }
	return 0;
}