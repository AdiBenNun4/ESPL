#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "LineParser.h"
#include <wait.h>
#include <signal.h>
int main()
{
    int stat;
    pid_t pid;
    if ((pid = fork()) == 0)
        while(1) ;
    else
    {
       // kill(pid, SIGINT);
        wait(&stat);
        if (WIFSIGNALED(stat))
            psignal(WTERMSIG(stat), "Child term due to");
    }
}