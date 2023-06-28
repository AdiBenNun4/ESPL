#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct state{
  char debug_mode;
  char file_name[128];
  int unit_size;
  unsigned char mem_buf[10000];
  size_t mem_count;
  char displayMode;
} state;

struct menu
{
  char *name;
  void (*fun)(struct state*);
};

void ToggleDebugMode(struct state* s){
  if (s->debug_mode==0) {
    s->debug_mode=1;
  }
  else if (s->debug_mode==1) {
    s->debug_mode=0;
    printf("%s","Debug flag now off\n");
  }
} 
void SetFileName(state* s){
  scanf( "%s", s->file_name);
  if(s->debug_mode==1){
    printf("Debug: file name set to '%s'\n", s->file_name);
  }
  
} 
void SetUnitSize(state* s){
  int unitsize;
  scanf("%d",&unitsize);
  if((unitsize==1) | (unitsize==2) | (unitsize==4)){
    s->unit_size = unitsize;
  }
  else{
    printf("%s","error : unit size is not valid");
  }
  if(s->debug_mode ==1){
    printf("Debug: set size to %d\n",s->unit_size);
  }
  
} 
void LoadIntoMemory(state* s){
  unsigned int location , length;
  if(strcmp(s->file_name,"")==0){
    printf("%s\n","error : file name is empty" );
  }else{
    FILE * f = fopen(s->file_name,"r");
    if(f == NULL) {
      printf("%s","error : file could not be opened");
    }else{
      scanf("%x",&location);
      scanf("%d",&length);
      if(s->debug_mode==1){
        printf("%X\n",location);
        printf("%d\n",length);
      }
      fseek(f,location,SEEK_SET);
      fread(s->mem_buf,s->unit_size,length,f);
      s->mem_count=length*s->unit_size;
      fclose(f);
    }
  }
    
  
} 
void ToggleDisplayMode(state* s){
  if (s->displayMode==0){
    s->displayMode=1;
    printf("Display flag now on, hexadecimal representation\n");
  }
  else{
    s->displayMode=0;
    printf("Display flag now off, decimal representation\n");
  } 
} 

int charsToInt (char* array, int index, int size){
  int ans=0;
  if (size==4)
  {
    ans=(array[index]*16777216)+(array[index+1]*65536);
    index=index+2;
  }
  if (size>1) {
    ans=ans+array[index]*256;
    index++;
  }
  return (ans+array[index]);
}



void MemoryDisplay(state* s){
  int u;
  int addr;
  char *buffer = (char *)calloc(10000* sizeof(char), sizeof(char));
  FILE *f;
  scanf("%X",&addr);
  scanf("%d",&u);
  if (addr==0) {
    if (s->displayMode==0){
      for (int i=0;i<(u*s->unit_size); i=i+s->unit_size){
          printf("%d\n",charsToInt((char*)s->mem_buf, i, s->unit_size));
      }
    }
    else {
      for (int i=0;i<(u*s->unit_size); i=i+s->unit_size){
          printf("%X\n",charsToInt((char*)s->mem_buf, i, s->unit_size));
      }
    }
  }
    else {
      f=fopen(s->file_name,"r");
      fseek(f,addr,SEEK_SET);
      fread(buffer,s->unit_size, u,f);
      if (s->displayMode==0){
      for (int i=0;i<(u*s->unit_size); i=i+s->unit_size){
          printf("%d\n",charsToInt(buffer, i, s->unit_size));
      }
    }
    else {
      for (int i=0;i<(u*s->unit_size); i=i+s->unit_size){
          printf("%X\n",charsToInt(buffer, i, s->unit_size));
      }
    }
    fclose(f);
  }
  free(buffer);
} 

void SaveIntoFile(state* s){
  int sourceAdress, targetLocation, length;
  scanf("%X",&sourceAdress);
  scanf("%X",&targetLocation);
  scanf("%d",&length);
  if (s->debug_mode==1){
    printf("sourceAdress:%x ,targetLocation:%x, length:%d",sourceAdress,targetLocation,length);
  }
  FILE * toWrite= fopen(s->file_name,"r+");
  
  if (toWrite!=NULL) {
    fseek(toWrite, targetLocation, SEEK_SET);
    if (ftell(toWrite)==-1) printf ("target location too big for file");
    else {
      if (sourceAdress==0){
    fwrite(s->mem_buf, 1, s->unit_size*length, toWrite);
  }
  else {
    fwrite((int *)targetLocation, 1, s->unit_size*length, toWrite);
  }
  fclose(toWrite);
    }
  }
  else printf("error opening file to write");
} 
void MemoryModify(state* s){
  int location, val;
  scanf("%X",&location);
  scanf("%X",&val);
  if (s->debug_mode==1) printf("location: %X , Value: %X", location, val);
  if (s->unit_size==1) {
    if (val<256 && location<10000) s->mem_buf[location]=val;
    else printf ("Values given too big");
  }
  else if (s->unit_size==2){
    if (val<(256*256) && location<9999) {
      s->mem_buf[location+1]=(val%256);
      s->mem_buf[location]=(val/256);
    }
  }
  else{
    if (location<9997){
      s->mem_buf[location+3]=(val%256);
      s->mem_buf[location+2]=((val/256)%256);
      s->mem_buf[location+1]=((val/(256*256))%256);
      s->mem_buf[location]=(val/(256*256*256));
    }//cc
    else {printf ("Values given too big");}
  }
    
printf ("%x",s->mem_buf[location]);
printf ("%x",s->mem_buf[location+1]);
printf ("%x",s->mem_buf[location+2]);
printf ("%x\n",s->mem_buf[location+3]);
} 
void Quit(state* s){
  if(s->debug_mode==1){
    printf("%s","Quitting\n");
  }
  if(s!=NULL){
      free(s);
  }
  exit(0);
} 

void menu(state* s)
{
   char num;
   int choice;
    struct menu menu[] = {{"Toggle Debug Mode", ToggleDebugMode}, {"Set File Name", SetFileName}, {"Set Unit Size", SetUnitSize}, {"Load Into Memory", LoadIntoMemory}
    ,{"Toggle Display Mode",ToggleDisplayMode},{"Memory Display",MemoryDisplay},{"Save Into File",SaveIntoFile},{"Memory Modify",MemoryModify}, {"Quit", Quit}, {NULL, NULL}};

  if(s->debug_mode==1){
    printf("%s","Debug flag now on\n");
    printf("%s","unit size :");
    printf("%d\n",s->unit_size);
    printf("file name: %s\n",s->file_name);
    printf("mam count: %d\n",s->mem_count);
  }
  printf("Please choose a function (ctrl^D for exit):\n");
        for (int i = 0; i < sizeof(menu) / sizeof(*menu) - 1; i++)
        {
            printf("%d.%s\n", i + 1, menu[i].name);
        }
   while ((num = fgetc(stdin)) != EOF)
    {
        if (num == '\n')
            (num = fgetc(stdin));
        fflush(stdout);
        choice = num - 49;
        if (choice >= 0 && choice < sizeof(menu) / sizeof(*menu) - 1)
        {
            menu[choice].fun(s);
        }
        else
        {
            exit(0);
        }
        if(s->debug_mode==1){
        printf("%s","Debug flag now on\n");
        printf("%s","unit size :");
        printf("%d\n",s->unit_size);
        printf("file name: %s\n",s->file_name);
        printf("mam count: %d\n",s->mem_count);
  }
        printf("Please choose a function (ctrl^D for exit):\n");
        for (int i = 0; i < sizeof(menu) / sizeof(*menu) - 1; i++)
        {
            printf("%d.%s\n", i + 1, menu[i].name);
        }
        fflush(stdout);
    }
    Quit(s);
}



int main(int argc, char **argv)
{
    struct state *s = (struct state *)malloc(sizeof(struct state));
    s->debug_mode = 0;
    s->unit_size = 1;
    s->mem_count = 0;
    s->displayMode=0;
    
    menu(s);
    return (0);
}