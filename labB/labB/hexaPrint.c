#include <stdio.h>
#include <stdlib.h>

void printHex(char * buffer, int length){
    for(int i=0 ; i<length ; i++){
        printf("%x ",buffer[i]);
    }
}


int main(int argc, char **argv)
{
    char *str = argv[1];

    FILE * f=fopen(str,"r");
    if(f==NULL)  printf("%s","File can't be created\n");
    char c;
    int count=0;
    while ((c=fgetc(f))!=EOF){
        count++;
    }
    char *buffer = (char*) calloc (count ,sizeof(char));
    fseek(f, 0 , SEEK_SET);
    fread(buffer ,  sizeof(char),count + 1  , f);
    printHex(buffer , count);
    fclose(f);
    free(buffer);
    return (0);
}