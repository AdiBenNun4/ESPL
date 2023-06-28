#include <stdio.h>
int func(char *arr ){
     int counter=0;
    for (int i=0; arr[i]!=NULL; i++){
    if (arr[i]<='9' && arr[i]>='0') counter++; 
    }
    return counter;
}

int main(int argc, char **argv)
{
   func(argv[1]);
    return 0;
}