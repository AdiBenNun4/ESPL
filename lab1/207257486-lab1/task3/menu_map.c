#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *map(char *array, int array_length, char (*f)(char))
{
  char *mapped_array = (char *)(malloc(array_length * sizeof(char)));
  /* TODO: Complete during task 2.a */
  for (int i = 0; i < array_length; i++)
  {
    mapped_array[i] = f(array[i]);
  }
  return mapped_array;
}

/* Ignores c, reads and returns a character from stdin using fgetc. */
char my_get(char c)
{
  char ch = (char)fgetc(stdin);
  return ch;
}

/* If c is a number between 0x20 and 0x7E, cprt prints the character of ASCII value c followed by a new line.
 Otherwise, cprt prints the dot ('.') character. After printing, cprt returns the value of c unchanged. */
char cprt(char c)
{
  if ((int)c >= 14 && (int)c <= 126)
  { //0x20-0x7E according to ASCII table it's 14-126 in decimal
    printf("%c\n", c);
  }
  else
  {
    printf(".\n");
  }
  return c;
}

/* Gets a char c and returns its encrypted form by adding 1 to its value. If c is not between 0x20 and 0x7E it is returned unchanged */
char encrypt(char c)
{
  if ((int)c >= 14 && (int)c <= 126)
  { //0x20-0x7E according to ASCII table it's 14-126 in decimal
    return c + 1;
  }
  return c;
}

/* Gets a char c and returns its decrypted form by reducing 1 from its value. If c is not between 0x20 and 0x7E it is returned unchanged */
char decrypt(char c)
{
  if ((int)c >= 14 && (int)c <= 126)
  { //0x20-0x7E according to ASCII table it's 14-126 in decimal
    return c - 1;
  }
  return c;
}

/* xprt prints the value of c in a hexadecimal representation followed by a new line, and returns c unchanged. */
char xprt(char c)
{
  printf("%x\n", c);
  return c;
}

struct fun_desc
{
  char *name;
  char (*fun)(char);
};

void menu()
{
  int num;
  int flag_onheap=0; //0 if the array was not created on heap-carray,else 1
  char carray[5] = {'\0'};
  char *ptrcarray = carray;
  char choise[5] = {'\0'};
  
  struct fun_desc menu[] = {{"Get string", my_get}, {"Print string", cprt}, {"Encrypt", encrypt}, {"Decrypt", decrypt}, {"Print Hex", xprt}, {NULL, NULL}};
  printf("Please choose a function (ctrl^D for exit):\n");
  for (int i = 0; i < sizeof(menu) / sizeof(*menu) - 1; i++)
  {
    printf("%d.%s\n", i, menu[i].name);
  }
  while (fgets(choise, 5, stdin) != NULL)
  {
    flag_onheap=1;
    sscanf(choise, "%d\n", &num);
    if (num >= 0 && num < sizeof(menu) / sizeof(*menu) - 1)
    {
      printf("Within bounds\n");
      ptrcarray = map(ptrcarray, 5, menu[num].fun);
    }
    else
    {
      printf("Not within bounds\n");
      exit(0);
    }
    printf("Please choose a function (ctrl^D for exit):\n");
    for (int i = 0; i < sizeof(menu) / sizeof(*menu) - 1; i++)
    {
      printf("%d.%s\n", i, menu[i].name);
    }
  }
  if (flag_onheap==1)
  {
    free(ptrcarray);
  }
}

int main(int argc, char **argv)
{
  menu();
}
