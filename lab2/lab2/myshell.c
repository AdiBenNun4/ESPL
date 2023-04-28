#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "LineParser.h"
#include <wait.h>

void execute(cmdLine *pCmdLine)
{
    int childPID;

    if (strcmp(pCmdLine->arguments[0], "quit") == 0)
        exit(0);
    else if (strcmp(pCmdLine->arguments[0], "cd") == 0)
    {
        chdir(pCmdLine->arguments[1]);
    }
    else if(strcmp(pCmdLine->arguments[0],"wake")==0){
        kill(atoi(pCmdLine->arguments[1]),SIGCONT);
    }
    else if(strcmp(pCmdLine->arguments[0],"kill")==0){
        kill(atoi(pCmdLine->arguments[1]),SIGTSTP);
    }
    else if ((childPID = fork()) == 0) 
    {//child's code
        if (pCmdLine->inputRedirect != NULL)
        {
            if(freopen(pCmdLine->inputRedirect, "r", stdin)==NULL) perror("can't change to this input");
        }
        if (pCmdLine->outputRedirect != NULL)
        {
            if(freopen(pCmdLine->outputRedirect, "w", stdout)==NULL) perror("can't change to this output");
        }
        int iserror = execvp(pCmdLine->arguments[0], pCmdLine->arguments);
        if (iserror == -1)
        {
            perror("failed to execute the program");
            _exit(1); // sends SIGCHLD - maby need to deal with it
        }
        freeCmdLines(pCmdLine);
    }
    else if (childPID == -1)
    {
        perror("failed fork the program");
        _exit(1);
    }
    else if (pCmdLine->blocking == 1)
    {
        waitpid(childPID, 0, P_ALL);
    }
}

int main(int argc, char **argv)
{
    int flag = 0;
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-d") == 0)
            flag = 1;
    }
    while (1)
    {
        if (flag == 1)
        {
            printf("PID: %d\n", getpid());
            printf("Executing command: %s\n", argv[0]);
        }
        char buffer[PATH_MAX];
        getcwd(buffer, PATH_MAX);
        printf("%s\n", buffer);
        char *line = (char *)malloc(sizeof(char));
        fgets(line, 2048 * sizeof(char), stdin);
        struct cmdLine *mycmd = parseCmdLines(line);
        execute(mycmd);
    }
    return (0);
}