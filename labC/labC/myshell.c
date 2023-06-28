#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "LineParser.h"
#include <wait.h>
#include <signal.h>

typedef struct process
{
    cmdLine *cmd;         /* the parsed command line*/
    pid_t pid;            /* the process id that is running the command*/
    int status;           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next; /* next process in chain */
} process;

#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0

void addProcess(process **process_list, cmdLine *cmd, pid_t pid)
{
    process *p = (process *)calloc(1,sizeof(process));

    p->cmd = cmd;
    p->pid = pid;
    p->status = 1;
    if (*process_list != NULL)
    {
        p->next = *process_list;
    }
    *process_list = p;
}

void updateProcessStatus(process *process_list, int stat)
{
    int isexit = waitpid(process_list->pid, &stat, WNOHANG | WUNTRACED);
    if (WIFSTOPPED(stat) | (isexit == 0 && process_list->status == 0))
    {
        process_list->status = 0;
    }
    else if (WIFEXITED(stat) | (isexit != 0))
    {
        process_list->status = -1;
    }
    else
    {
        process_list->status = 1;
    }
}

void updateProcessList(process **process_list)
{
    int stat;
    if (*process_list != NULL)
    {
        process *cur = *process_list;
        while (cur != NULL)
        {
            updateProcessStatus(cur, stat);
            cur = cur->next;
        }
    }
}

void printProcessList(process **process_list)
{
    process *cur = *process_list;
    process *prev = NULL;
    updateProcessList(process_list);
    printf("%s\n", "PID          Command      STATUS");
    while (cur != NULL)
    {
        printf("%d          ", cur->pid);
        printf("%s      ", cur->cmd->arguments[0]);
        int stat = cur->status;
        if (stat == 1)
        {
            printf("%s\n", "RUNNING");
            prev = cur;
            cur = cur->next;
        }
        else if (stat == 0)
        {
            printf("%s\n", "SUSPENDED");
            prev = cur;
            cur = cur->next;
        }
        else
        {
            printf("%s\n", "TERMINATED");
            if (prev == NULL)
            {
                if (cur->next == NULL)
                    *process_list = NULL;
                else
                    *process_list = cur->next;
            }
            else
            {
                prev->next = cur->next;
            }
            process *temp = cur;
            cur = cur->next;
            if (temp != NULL)
            {
                if (temp->cmd->arguments!=NULL){
                  freeCmdLines(temp->cmd);
                }
                free(temp);
            }
        }
    }
}

void freeProcessList(process *process_list)
{
    if (process_list != NULL)
    {
        if (process_list->next != NULL)
        {
            freeProcessList(process_list->next);
        }
        if (process_list->cmd != NULL)
            freeCmdLines(process_list->cmd);
        free(process_list);
    }
}

void executePipe(cmdLine *mycmd, process **pList)
{
    {
        {
            char *command1 = mycmd->arguments[0];
            char *command2 = mycmd->next->arguments[0];
            int child1Id, child2Id;
            if (mycmd->outputRedirect != NULL)
                fputs("illigal changing the output", stderr);
            if (mycmd->next->inputRedirect != NULL)
                fputs("illigal changing the input", stderr);
            int mypipe[2];
            if (pipe(mypipe) < 0)
                _exit(1);
            if ((child1Id = fork()) == 0)
            { // child1 code
                close(1);
                dup(mypipe[1]);
                close(mypipe[1]);
                execvp(command1, mycmd->arguments);
            }
            else
            { // parent code
                addProcess(pList, mycmd, child1Id);
                close(mypipe[1]);
                if ((child2Id = fork()) == 0)
                { // child2 code
                    close(0);
                    dup(mypipe[0]);
                    close(mypipe[0]);
                    execvp(command2, mycmd->next->arguments);
                }
                else
                {
                    addProcess(pList, mycmd->next, child2Id);
                    close(mypipe[0]);
                }
            }
            waitpid(-1, 0, 0);
        }
    }
}

void execute(cmdLine *pCmdLine, process **pList)
{
    int childPID;
    if (strcmp(pCmdLine->arguments[0], "cd") == 0)
    {
        chdir(pCmdLine->arguments[1]);
        freeCmdLines(pCmdLine);
    }
    else if (strcmp(pCmdLine->arguments[0], "wake") == 0)
    {
        kill(atoi(pCmdLine->arguments[1]), SIGCONT);
        process *cur = *pList;
        while (cur != NULL)
        {
            if (cur->pid == atoi(pCmdLine->arguments[1]))
            {
                cur->status = 1;
                break;
            }
            cur = cur->next;
        }
        freeCmdLines(pCmdLine);
    }
    else if (strcmp(pCmdLine->arguments[0], "kill") == 0)
    {
        kill(atoi(pCmdLine->arguments[1]), SIGINT);
        freeCmdLines(pCmdLine);
    }
    else if (strcmp(pCmdLine->arguments[0], "suspend") == 0)
    {
        kill(atoi(pCmdLine->arguments[1]), SIGTSTP);
        freeCmdLines(pCmdLine);
    }
    else if (strcmp(pCmdLine->arguments[0], "procs") == 0)
    {
        printProcessList(pList);
        freeCmdLines(pCmdLine);
    }
    else if (pCmdLine->next != NULL)
    {
        executePipe(pCmdLine, pList);
    }
    else if ((childPID = fork()) == 0)
    { // child's code
        if (pCmdLine->inputRedirect != NULL)
        {
            if (freopen(pCmdLine->inputRedirect, "r", stdin) == NULL)
                perror("can't change to this input");
        }
        if (pCmdLine->outputRedirect != NULL)
        {
            if (freopen(pCmdLine->outputRedirect, "w", stdout) == NULL)
                perror("can't change to this output");
        }
        int iserror = execvp(pCmdLine->arguments[0], pCmdLine->arguments);
        if (iserror == -1)
        {
            perror("failed to execute the program");
            _exit(1); // sends SIGCHLD - maby need to deal with it
        }
    }
    else if (childPID == -1)
    {
        perror("failed fork the program");
        _exit(1);
    }
    else if (pCmdLine->blocking == 1)
    {
        addProcess(pList, pCmdLine, childPID);
        waitpid(childPID, 0, 0);
    }
    else
    {
        addProcess(pList, pCmdLine, childPID);
    }
}

void freehistory(int count, char **history)
{
    if (history != NULL)
    {
        int maxsize = count;
        if (count > 20)
            maxsize = 20;
        for (int i = 0; i < maxsize; i++)
        {
            if (history[i] != NULL)
                free(history[i]);
        }
        free(history);
    }
}

void printHistory(char **history, int count)
{
    printf("%s\n", "oldest\n");
    if (count > 20)
    {
        for (int i = count % 20; i < 20; i++)
        {
            printf("%d ", i - (count % 20));
            printf("%s", history[i]);
        }
        for (int i = 0; i < count % 20; i++)
        {
            printf("%d ", 20 - count % 20 + i);
            printf("%s", history[i]);
        }
    }
    else
    {
        for (int i = 0; i < count; i++)
        {
            printf("%d ", i);
            printf("%s", history[i]);
        }
    }

    printf("%s\n", "newest\n");
}

int main(int argc, char **argv)
{
    process **pList = (process **)malloc(sizeof(process));
    *pList=NULL;
    char **history = (char **)calloc(20, 4);
    *history = NULL;
    int count = 0;
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
        printf("%s>", buffer);
        char *line = (char *)malloc(20000 * sizeof(char));
        fgets(line, 20000 * sizeof(char), stdin);
        struct cmdLine *mycmd = parseCmdLines(line);
        if (mycmd != NULL)
        {
            if (strcmp(mycmd->arguments[0], "quit") == 0)
            {
                if (line != NULL)
                {
                    free(line);
                }
                freeProcessList(*pList);
                if (pList != NULL)
                {
                    free(pList);
                }
                freehistory(count, history);
                if (mycmd != NULL)
                {
                    freeCmdLines(mycmd);
                }
                exit(0);
            }
            else if (strcmp(mycmd->arguments[0], "history") == 0)
            {
                printHistory(history, count);
                if (mycmd != NULL)
                {
                    freeCmdLines(mycmd);
                }
                if (line != NULL)
                {
                    free(line);
                }
            }
            else if (strcmp(mycmd->arguments[0], "!!") == 0)
            {
                struct cmdLine *tempCMD = parseCmdLines(history[(count - 1) % 20]);
                execute(tempCMD, pList);
                if (tempCMD != NULL)
                {
                    freeCmdLines(tempCMD);
                }
                if (mycmd != NULL)
                {
                    freeCmdLines(mycmd);
                }
                if (line != NULL)
                {
                    free(line);
                }
            }
            else if (mycmd->arguments[0][0] == '!')
            {
                int n = 0;
                for (int i = 1; (mycmd->arguments[0][i] <= 57 && mycmd->arguments[0][i] >= 48); i++)
                {
                    n = n * 10 + (mycmd->arguments[0][i] - 48);
                }
                if (n > 0 && n <= 20 && n <= count)
                {
                    struct cmdLine *tempCMD = parseCmdLines(history[(count - n) % 20]);
                    execute(tempCMD, pList);
                    if (tempCMD != NULL)
                    {
                        freeCmdLines(tempCMD);
                    }
                }
                else
                    perror("illigal command");
                if (mycmd != NULL)
                {
                    freeCmdLines(mycmd);
                }

                if (line != NULL)
                {
                    free(line);
                }
            }
            else
            {
                execute(mycmd, pList);
                if (count > 19)
                {
                    free(history[count % 20]);
                }
                history[count % 20] = line;
                count = count + 1;
            }
        }
    }

    return (0);
}