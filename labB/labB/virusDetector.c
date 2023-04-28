#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct virus
{
    unsigned short SigSize;
    char virusName[16];
    unsigned char *sig;
} virus;

typedef struct link
{
    struct link *nextVirus;
    virus *vir;
} link;

struct menu
{
    char *name;
    struct link *(*fun)(struct link *, FILE *, char *, unsigned int *);
};

virus *readVirus(FILE *f)
{
    char str[2];
    fread(str, sizeof(char), 2, f);
    unsigned short SigSize = str[0] + str[1] * 256;
    char *virusName = (char *)calloc(16, sizeof(char));
    fread(virusName, sizeof(char), 16, f);
    unsigned char *sig = (unsigned char *)calloc(SigSize, sizeof(unsigned char));
    fread(sig, sizeof(char), SigSize, f);
    struct virus *myvirus = (struct virus *)malloc(sizeof(struct virus));
    myvirus->SigSize = SigSize;
    myvirus->sig = sig;
    strcpy(myvirus->virusName, virusName);
    free(virusName);
    return myvirus;
}

struct link *list_append(struct link *virus_list, struct virus *data)
{
    if (virus_list->vir == NULL) //check if the list already has at least one virus
    { 
        virus_list->vir = data;
        virus_list->nextVirus = NULL;
        return virus_list;
    }
    else
    {
        struct link *newstart = (struct link *)malloc(sizeof(struct link));
        newstart->vir = data;
        newstart->nextVirus = virus_list;
        return newstart;
    }
}

link *readVirusHelper(struct link *list, FILE *notinUsefile, char *notinuse, unsigned int *notused)
{
    char str[4];
    char visb[4] = {'V', 'I', 'S', 'B'};
    char visl[4] = {'V', 'I', 'S', 'L'};
    char c;
    FILE *f;
    char fileName[100];
    printf("%s\n", "enter the name of the file: ");
    scanf("%s", fileName);
    f = fopen(fileName, "r");
    if (f == NULL)
    {
        printf("%s", "There is not such file");
        exit(1);
    }
    fread(str, sizeof(char), 4, f);
    int flag = 0; //Are any of the first four characters in the file wrong
    for (int i = 0; i < 4; i++)
    {
        if (visl[i] != str[i] && visb[i] != str[i])
            flag = 1;
    }
    if (flag == 1)
    {
        printf("%s\n", "The magic number is incorrect");
    }
    else
    {
        while ((c = fgetc(f)) != EOF)
        {
            ungetc(c, f);
            struct virus *myvirus = readVirus(f);
            list = list_append(list, myvirus);
        }
    }
    fclose(f);
    return list;
}

void printVirus(struct virus *virus, FILE *output)
{
    char buffer1[16];
    char buffer2[3];
    fputs("Virus name: ", output);
    fputs(virus->virusName, output);
    fputs("\nVirus size: ", output);
    int size = virus->SigSize;
    sprintf(buffer1, "%d", size);
    fputs(buffer1, output);
    fputs("\nsignature:\n", output);
    unsigned char *p = virus->sig;

    for (int i = 0; i < size; i++)
    {
        sprintf(buffer2, "%02X", *p);
        fputs(buffer2, output);
        fputs(" ", output);
        p++;
    }
    fputs("\n\n", output);
}

void list_free(struct link *virus_list)
{
    if(virus_list!=NULL){
        if (virus_list->nextVirus != NULL) list_free(virus_list->nextVirus);
        if (virus_list->vir != NULL) {
            if (virus_list->vir->sig != NULL) free(virus_list->vir->sig);
            free(virus_list->vir);
        }
        free(virus_list);
    }
}

link *list_print(struct link *virus_list, FILE *f, char *notinused, unsigned int *notused)
{
    if (f == NULL)
        f = stdout;
    link *temp = virus_list;
    if (temp != NULL)
    {
        printVirus(temp->vir, f);
        temp = temp->nextVirus;
        list_print(temp, f, notinused, notused);
    }
    return virus_list;
}

void printHex(unsigned char *buffer, int length)
{
    for (int i = 0; i < length; i++)
    {
        printf("%02X ", buffer[i]);
    }
}

void detect_virus(char *buffer, unsigned int size, link *virus_list, unsigned int *viruslocation)
{
    link *templist = virus_list;
    char *bufferptr = buffer;
    virus *cur;
    unsigned int count = 0;
    while (count <= size && templist != NULL)
    { //loop every virus
        cur = templist->vir;
        while (count <= size)
        { //loop on the buffer for the same virus
            int result = memcmp(cur->sig, bufferptr, cur->SigSize);
            if (result == 0)
            {
                *viruslocation = count;
                printf("starting byte location in the suspected file in HEX: %02X\n", count);
                printf("virus name: %s\n", cur->virusName);
                printf("the size of virus signature: %d\n", cur->SigSize);
                printf("\n");
                break;
            }
            else
            {
                bufferptr++;
                count++;
            }
        }
        templist = templist->nextVirus;
        count = 0;
        bufferptr = buffer;
    }
}

link *detectHelper(struct link *virus_list, FILE *notinused, char *detectFileName, unsigned int *viruslocation)
{
    FILE *detectFrom;
    if (detectFileName != NULL)
    {
        detectFrom = fopen(detectFileName, "r");
        if(detectFrom==NULL) exit(1);
        char *buffer = (char *)calloc(1024* sizeof(char), sizeof(char));
        fseek(detectFrom, 0, SEEK_END);
        unsigned int sizeDF = ftell(detectFrom);
        rewind(detectFrom);
        unsigned int size = sizeDF;
        if (size > 1024 * sizeof(char))
            size = 1024 * sizeof(char);
        fread(buffer, sizeof(char), size, detectFrom);
        detect_virus(buffer, size, virus_list, viruslocation);
        free(buffer);
        fclose(detectFrom);
    }

    return virus_list;
}

link *Quit(struct link *virus_list, FILE *notinused, char *notInUsed, unsigned int *viruslocation)
{
    list_free(virus_list);
    if (viruslocation!=NULL) free(viruslocation);
    exit(0);
    return virus_list;
}

void neutralize_virus(char *fileName, int signatureOffset)
{
    FILE *tofix = fopen(fileName, "r+");
    if (tofix != NULL)
    {
        fseek(tofix, signatureOffset, SEEK_SET);
        char c = 195;
        char *temp = &c;
        fwrite(temp, sizeof(char), 1, tofix);
        rewind(tofix);
        fclose(tofix);
    }
    else
    {
        exit(1);
    }
}

link *fixFile(struct link *virus_list, FILE *notinused, char *fileName, unsigned int *viruslocation)
{
    neutralize_virus(fileName, *viruslocation);
    return virus_list;
}

void menu(char *detectFileName)
{
    unsigned int *virusLocation = (unsigned int *)malloc(sizeof(unsigned int));
    char num;
    int flag_onheap = 0; //0 if the array was not created on heap-carray,else 1
    int choice;
    struct link *list = (struct link *)malloc(sizeof(struct link));
    FILE *f = NULL;
    struct menu menu[] = {{"Load signatures", readVirusHelper}, {"Print signatures", list_print}, {"Detect viruses", detectHelper}, {"Fix file", fixFile}, {"Quit", Quit}, {NULL, NULL}};
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
        flag_onheap = 1;
        if (choice >= 0 && choice < sizeof(menu) / sizeof(*menu) - 1)
        {
            list = menu[choice].fun(list, f, detectFileName, virusLocation);
        }
        else
        {
            exit(0);
        }
        printf("Please choose a function (ctrl^D for exit):\n");
        for (int i = 0; i < sizeof(menu) / sizeof(*menu) - 1; i++)
        {
            printf("%d.%s\n", i + 1, menu[i].name);
        }
        fflush(stdout);
    }
    if (flag_onheap == 1)
    {
        free(list);
    }
    Quit(list, f, detectFileName, virusLocation);
}

int main(int argc, char **argv)
{
    if (argc >= 2)
        menu(argv[1]);
    else
        menu(NULL);

    /*Part 1A****************/
    // FILE *f = fopen("signatures-L", "r");
    // char str[4];
    // char *virb = "VISB";
    // char *virl = "VISL";
    // char c;
    // fread(str, sizeof(char), 4, f);
    // if (strcmp(str, virl) != 0 && strcmp(str, virb) != 0)
    // {
    //     printf("%s\n", "The magic number is incorrect");
    // }
    // else
    // {
    //     while ((c = fgetc(f)) != EOF)
    //     {
    //         ungetc(c, f);
    //         virus *myvirus = readVirus(f);
    //         printVirus(myvirus, stdout);
    //     }
    // }

    return (0);
}