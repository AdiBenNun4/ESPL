#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "LineParser.h"
#include <wait.h>

int main(int argc, char **argv)
{
    /**************Part 1******************/
    char *strings1[3] = {"ls","-l",NULL};
    char *strings2[4] = {"tail","-n","2",NULL};
    int mypipe[2];
    FILE * f= fopen("Tests","w");
    if(pipe(mypipe) < 0) _exit(1);
    int child1pid,child2pid;
    fputs("(parent_process>forking…)\n",f);
    if((child1pid=fork()) == 0){ //child1 code
        fputs("(child1>redirecting stdout to the write end of the pipe…)\n",f);
       fclose(stdout);
       dup(mypipe[1]);
       close(mypipe[1]);
       fputs("(child1>going to execute cmd: …)\n",f);
        fflush(f);
       execvp("ls",strings1);
    }
    else{ //parent code
        fputs("(parent_process>created process with id:",f);
        fprintf(f,"%d\n",getpid());
        fputs("(parent_process>closing the write end of the pipe…)\n",f);
        close(mypipe[1]);
       if((child2pid=fork()) == 0){ //child2 code
            fputs("(child2>redirecting stdout to the write end of the pipe…)\n",f);
            fclose(stdin);
            dup(mypipe[0]);
            close(mypipe[0]);
            fputs("(child2>going to execute cmd: …)\n",f);
            fflush(f);
            execvp("tail",strings2);
       }
       else{
        fputs("(parent_process>closing the read end of the pipe…)\n",f);
        close(mypipe[0]);
       }
    }
    fputs("(parent_process>waiting for child processes to terminate…)\n",f);
    waitpid(-1,0,0);
    fputs("(parent_process>exiting…)\n",f);
    fclose(f);
    
    /***************************End Of part One***************/
   
	return 0;
}