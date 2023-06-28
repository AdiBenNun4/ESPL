
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <elf.h>
#include <sys/mman.h>
#include <fcntl.h>
typedef struct info{
  char debug_mode;
  int numMappedFiles;
  int fds[2];
  int lengths[2];
  char* filename1;
  char* filename2;
  void* mappedAdresses1;
  void* mappedAdresses2;
} info;

struct menu
{
  char *name;
  void (*fun)(struct info*);
};

void ToggleDebugMode(struct info* info){
  if (info->debug_mode==0) {
    info->debug_mode=1;
    printf("%s","Debug flag now on\n");
  }
  else if (info->debug_mode==1) {
    info->debug_mode=0;
    printf("%s","Debug flag now off\n");
  }
} 
void ExamineElfFile(struct info* info){
    if (info->numMappedFiles >= 2) { //need to be sent
        printf("Maximum number of mapped files reached.\n");
        return;
    }

    char filename[256];
    printf("Enter ELF filename: ");
    scanf("%s", filename);
    FILE *file=fopen(filename, "r+");
        if (file==NULL){
        perror("Failed to open file");
        return;
        }
    int  fd=fileno(file);
    if (fd == -1) {
        perror("Error opening file");
        return;
    }
    if (fseek(file,0,SEEK_END)!=0){
        perror("Failed to move file pointer");
        return;
        }
        int length=ftell(file);
        fseek(file, 0, SEEK_SET);

    void* mappedAddr = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);
    if (info->numMappedFiles==0) {
        info->mappedAdresses1=mappedAddr;
        info->lengths[0]=length;
        strcpy(info->filename1, filename);
    }
    else {
        info->mappedAdresses2=mappedAddr;  //verify doesn't need dereference
        info->lengths[1]=length;
        strcpy(info->filename2, filename);
        }   
    if (mappedAddr == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        return;
    }

    info->fds[info->numMappedFiles] = fd;
    info->numMappedFiles++;

    Elf32_Ehdr* elfHeader = (Elf32_Ehdr*)mappedAddr;
    printf("Magic numbers: %c%c%c\n", elfHeader->e_ident[1], elfHeader->e_ident[2], elfHeader->e_ident[3]);
    printf("Data Encoding Scheme: %s\n",
           (elfHeader->e_ident[EI_DATA] == ELFDATA2LSB) ? "little endian" :
           (elfHeader->e_ident[EI_DATA] == ELFDATA2MSB) ? "big endian" :
           "Unknown");
    printf("Entry point: %x\n", elfHeader->e_entry);
    printf("Section header table offset: %d\n", elfHeader->e_shoff);
    printf("Number of section header entries: %d\n", elfHeader->e_shnum);
    printf("Size of each section header entry: %d\n", elfHeader->e_shentsize);
    printf("Program header table offset: %d\n", elfHeader->e_phoff);
    printf("Number of program header entries: %d\n", elfHeader->e_phnum);
    printf("Size of each program header entry: %d\n", elfHeader->e_phentsize);
} 
void PSNhelper(void* mappedAdresses1, struct info* info, char *filename, int index){
        Elf32_Ehdr* elfHeader = (Elf32_Ehdr*)info->mappedAdresses1;
        Elf32_Shdr* sectionHeaders = (Elf32_Shdr*)((char*)info->mappedAdresses1 + elfHeader->e_shoff);
        Elf32_Shdr* sectionHeaderStringTable = &sectionHeaders[elfHeader->e_shstrndx];
        const char* sectionNames = (char*)info->mappedAdresses1 + sectionHeaderStringTable->sh_offset;

        printf("File: %s\n", info->filename1);

        printf("[index] section_name section_address section_offset section_size  section_type\n");
        for (int i = 0; i < elfHeader->e_shnum; i++) {
            Elf32_Shdr* sectionHeader = &sectionHeaders[i];
            const char* sectionName = &sectionNames[sectionHeader->sh_name];
            printf("[%d] %s 0x%08x 0x%08x %d  %d\n", i, sectionName, sectionHeader->sh_addr,
                   sectionHeader->sh_offset, sectionHeader->sh_size, sectionHeader->sh_type);
        }

          if (info->debug_mode==1) {
            printf("Debug information for file: %s\n", filename);
            printf("e_shstrndx: %d\n", elfHeader->e_shstrndx);
            printf("Section name offsets:\n");
            for (int i = 0; i < elfHeader->e_shnum; i++) {
                Elf32_Shdr* sectionHeader = &sectionHeaders[i];
                const char* sectionName = &sectionNames[sectionHeader->sh_name];
                printf("[%d] %s: 0x%08x\n", i, sectionName, sectionHeader->sh_name);
            }
        }
    }
void PrintSectionNames(struct info* info){
  if (info->numMappedFiles == 0) {
        printf("No ELF files mapped.\n");
        return;
    }
    if (info->numMappedFiles>0) PSNhelper(info->mappedAdresses1,info, info->filename1, 0);
    if (info->numMappedFiles>1) PSNhelper(info->mappedAdresses2,info, info->filename2, 1);
} 

void PsymHelper(void* mappedAdresses1, struct info* info, char *filename, int index){
     
        Elf32_Ehdr* elfHeader = (Elf32_Ehdr*)mappedAdresses1;
        Elf32_Shdr* sectionHeaders = (Elf32_Shdr*)((char*)mappedAdresses1 + elfHeader->e_shoff);
        Elf32_Shdr* symbolTableHeader = NULL;
        Elf32_Shdr* stringTableHeader = NULL;
        const char* stringTable = NULL;

        for (int i = 0; i < elfHeader->e_shnum; i++) {
            Elf32_Shdr* sectionHeader = &sectionHeaders[i];
            if (sectionHeader->sh_type == SHT_SYMTAB) {
                symbolTableHeader = sectionHeader;
            } else if (sectionHeader->sh_type == SHT_STRTAB && i != elfHeader->e_shstrndx) {
                stringTableHeader = sectionHeader;
                stringTable = (char*)mappedAdresses1 + stringTableHeader->sh_offset;
            }
        }

        Elf32_Sym* symbolTable = (Elf32_Sym*)((char*)mappedAdresses1 + symbolTableHeader->sh_offset);

        printf("File: %s\n", filename);

        printf("[index] value section_index section_name symbol_name\n");
        for (int i = 0; i < symbolTableHeader->sh_size / sizeof(Elf32_Sym); i++) {
            Elf32_Sym* symbol = &symbolTable[i];
            const char* symbolName = stringTable + symbol->st_name;

            const char* sectionName = "";
            if (symbol->st_shndx != SHN_UNDEF && symbol->st_shndx < elfHeader->e_shnum) {
                Elf32_Shdr* symbolSectionHeader = &sectionHeaders[symbol->st_shndx];
                sectionName = (char*)mappedAdresses1 + symbolSectionHeader->sh_name+stringTableHeader->sh_offset;
            }

            printf("[%d] 0x%08x %d %s %s\n", i, symbol->st_value, symbol->st_shndx, sectionName, symbolName);
        }

        if (info->debug_mode==1) {
            printf("Debug information for file: %s\n", filename);
            printf("Symbol table size: %u\n", symbolTableHeader->sh_size);
            printf("Number of symbols: %u\n", symbolTableHeader->sh_size / sizeof(Elf32_Sym));
        }
}

void PrintSymbols(struct info* info){
if (info->numMappedFiles == 0) {
        printf("No ELF files mapped.\n");
        return;
    }
    if (info->numMappedFiles>0) PsymHelper(info->mappedAdresses1,info, info->filename1, 0);
    if (info->numMappedFiles>1) PsymHelper(info->mappedAdresses2,info, info->filename2, 1);
} 
void CheckFilesForMerge(struct info* info){
    if (info->numMappedFiles != 2) {
        printf("Error: Two ELF files must be opened and mapped.\n");
        return;
    }

    Elf32_Ehdr* elfHeader1 = (Elf32_Ehdr*)info->mappedAdresses1;
    Elf32_Ehdr* elfHeader2 = (Elf32_Ehdr*)info->mappedAdresses2;

    Elf32_Shdr* sectionHeaders1 = (Elf32_Shdr*)((char*)info->mappedAdresses1 + elfHeader1->e_shoff);
    Elf32_Shdr* sectionHeaders2 = (Elf32_Shdr*)((char*)info->mappedAdresses2 + elfHeader2->e_shoff);

    Elf32_Shdr* symbolTableHeader1 = NULL;
    Elf32_Shdr* symbolTableHeader2 = NULL;
    Elf32_Sym* symbolTable1 = NULL;
    Elf32_Sym* symbolTable2 = NULL;
    const char* stringTable1 = NULL;
    const char* stringTable2 = NULL;

    // Find the symbol table and string table sections in the first ELF file
    for (int i = 0; i < elfHeader1->e_shnum; i++) {
        Elf32_Shdr* sectionHeader = &sectionHeaders1[i];
        if (sectionHeader->sh_type == SHT_SYMTAB) {
            symbolTableHeader1 = sectionHeader;
            symbolTable1 = (Elf32_Sym*)((char*)info->mappedAdresses1 + symbolTableHeader1->sh_offset);
        } else if (sectionHeader->sh_type == SHT_STRTAB && i != elfHeader1->e_shstrndx) {
            stringTable1 = (char*)info->mappedAdresses1 + sectionHeader->sh_offset;
        }
    }

    // Find the symbol table and string table sections in the second ELF file
    for (int i = 0; i < elfHeader2->e_shnum; i++) {
        Elf32_Shdr* sectionHeader = &sectionHeaders2[i];
        if (sectionHeader->sh_type == SHT_SYMTAB) {
            symbolTableHeader2 = sectionHeader;
            symbolTable2 = (Elf32_Sym*)((char*)info->mappedAdresses2 + symbolTableHeader2->sh_offset);
        } else if (sectionHeader->sh_type == SHT_STRTAB && i != elfHeader2->e_shstrndx) {
            stringTable2 = (char*)info->mappedAdresses2 + sectionHeader->sh_offset;
        }
    }

    if (!symbolTableHeader1 || !symbolTableHeader2) {
        printf("Error: Two ELF files must have exactly one symbol table each.\n");
        return;
    }

    printf("Checking for symbol merge between files.\n");

    // Iterate over symbols in the first ELF file
    for (int i = 1; i < symbolTableHeader1->sh_size / sizeof(Elf32_Sym); i++) {
        Elf32_Sym* symbol1 = &symbolTable1[i];
        const char* symbolName1 = stringTable1 + symbol1->st_name;
        if (symbol1->st_shndx == SHN_UNDEF) {
            // Check if symbol is undefined in the second ELF file
            int found = 0;
            for (int j = 1; j < symbolTableHeader2->sh_size / sizeof(Elf32_Sym); j++) {
                Elf32_Sym* symbol2 = &symbolTable2[j];
                const char* symbolName2 = stringTable2 + symbol2->st_name;

                if (strcmp(symbolName1, symbolName2) == 0 && symbol2->st_shndx != SHN_UNDEF) {
                    found = 1;
                    break;
                }
            }

            if (!found) {
                printf("Symbol %s undefined.\n", symbolName1);
            }
        } else {
            // Check if symbol is defined in the second ELF file
            for (int j = 1; j < symbolTableHeader2->sh_size / sizeof(Elf32_Sym); j++) {
                Elf32_Sym* symbol2 = &symbolTable2[j];
                const char* symbolName2 = stringTable2 + symbol2->st_name;
                //printf("%s %s %d %d\n", symbolName1, symbolName2, i, j);
                if (strcmp(symbolName1, symbolName2) == 0 && symbol2->st_shndx != SHN_UNDEF && *symbolName1!=NULL) {
                    printf("Symbol %s multiply defined.\n", symbolName1);
                }
            }
        }
    }

    printf("Symbol merge check completed.\n");
}

void MergeElfFiles (struct info* info){
  int output_fd = open("out.ro", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    off_t output_size = info->lengths[0] + info->lengths[1];
    ftruncate(output_fd, output_size);

    void* output_data = mmap(NULL, output_size, PROT_READ | PROT_WRITE, MAP_SHARED, output_fd, 0);

    // Copy ELF header
    memcpy(output_data, info->mappedAdresses1, sizeof(Elf32_Ehdr));

    // Get section header offsets and sizes
    Elf32_Ehdr* header1 = (Elf32_Ehdr*)info->mappedAdresses1;
    Elf32_Ehdr* header2 = (Elf32_Ehdr*)info->mappedAdresses2;
    Elf32_Shdr* section_headers1 = (Elf32_Shdr*)(info->mappedAdresses1 + header1->e_shoff);
    Elf32_Shdr* section_headers2 = (Elf32_Shdr*)(info->mappedAdresses2 + header2->e_shoff);
    Elf32_Shdr* output_section_headers = (Elf32_Shdr*)(output_data + header1->e_shoff);

    // Copy section names and types from file1
    Elf32_Shdr* section_header_names1 = &section_headers1[header1->e_shstrndx];
    void* section_names1 = info->mappedAdresses1 + section_header_names1->sh_offset;
    void* output_section_names = output_data + section_header_names1->sh_offset;
    memcpy(output_section_names, section_names1, section_header_names1->sh_size);

    // Merge sections
    for (int i = 0; i < header1->e_shnum; i++) {
        Elf32_Shdr section_header1 = section_headers1[i];
        Elf32_Shdr section_header2 = section_headers2[i];
        if (section_header1.sh_type == SHT_PROGBITS) {
            size_t section_size1 = section_header1.sh_size;
            size_t section_size2 = section_header2.sh_size;
            size_t merged_size = section_size1 + section_size2;

            // Copy section contents from file1
            void* section_contents1 = info->mappedAdresses1 + section_header1.sh_offset;
            void* output_section_contents1 = output_data + section_header1.sh_offset;
            memcpy(output_section_contents1, section_contents1, section_size1);

            // Copy section contents from file2
            void* section_contents2 = info->mappedAdresses2 + section_header2.sh_offset;
            void* output_section_contents2 = output_data + section_header1.sh_offset + section_size1;
            memcpy(output_section_contents2, section_contents2, section_size2);

            // Update section header with merged size and offset
            output_section_headers[i].sh_size = merged_size;
            output_section_headers[i].sh_offset += section_header1.sh_offset + section_size1;

            // Copy section name and type from file1
            output_section_headers[i].sh_name = section_header1.sh_name;
            output_section_headers[i].sh_type = section_header1.sh_type;
        }
    }

    // Clean up
    munmap(output_data, output_size);
    close(output_fd);
}

void Quit(struct info* info){
    if(info!=NULL)
    {
    if (info->numMappedFiles>0) munmap(info->mappedAdresses1, info->lengths[0]);
    if (info->numMappedFiles>1) munmap(info->mappedAdresses2, info->lengths[1]);
    if (info->filename1!=NULL) free(info->filename1);
    if (info->filename2!=NULL) free(info->filename2);
    free(info);
    }
    printf("Exiting\n");
    exit(0);
} 

void menu()
{
    struct info *info = (struct info *)malloc(sizeof(struct info));
   char num;
   int choice;
   info->debug_mode=0;
   info->numMappedFiles=0;
   info->filename1=(char *)calloc(256, sizeof(char));
   info->filename2=(char *)calloc(256, sizeof(char));
    struct menu menu[] = {{"Toggle Debug Mode", ToggleDebugMode}, {"Examine ELF File", ExamineElfFile}, {"Print Section Names", PrintSectionNames}, {"Print Symbols", PrintSymbols}
    ,{"Check Files For Merge",CheckFilesForMerge},{"Merge Elf Files",MergeElfFiles}, {"Quit", Quit}, {NULL, NULL}};
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
            menu[choice].fun(info);
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
    Quit(info);
}



int main(int argc, char **argv)
{
    menu();
    return (0);
}