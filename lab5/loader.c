#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <elf.h>
#include <sys/mman.h>
extern int startup(int argc, char **argv, void (*start)());

void printFlags(Elf32_Phdr *phdr){
    if (phdr->p_flags & PF_R) printf("R ");
    if (phdr->p_flags & PF_W) printf("W ");
    if (phdr->p_flags & PF_X) printf("E ");
    int flags = 0;
    if (phdr->p_flags & PF_R) flags += PROT_EXEC ;
    if (phdr->p_flags & PF_W) flags += PROT_WRITE;
    if (phdr->p_flags & PF_X) flags += PROT_READ ;
   // if (phdr->p_flags & PF_R) flags |= MAP_PRIVATE;
    //else flags=0;
    printf("0x%x ", flags);
}

void printElfL(Elf32_Phdr *phdr) {
    printf("Type Offset VirtAddr PhysAddr FileSiz MemSiz Flg Align \n");
    printf("%d 0x%06X 0x%08X 0x%08X 0x%05X 0x%05X ", phdr->p_type, phdr->p_offset, phdr->p_vaddr, phdr->p_paddr, phdr->p_filesz, phdr->p_memsz);
    printFlags (phdr);
    printf("0x%x\n", phdr->p_align);
}

void debugFunc(Elf32_Phdr *phdr, int index) {
    printf("Program header number %d at address %p\n", index, phdr);
}

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int),int arg, char **args, int argc) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map_start;
    Elf32_Phdr *phdr = (Elf32_Phdr *)(map_start + ehdr->e_phoff);
        

    for (int i = 0; i < ehdr->e_phnum; i++) {
        printElfL(&phdr[i]);
        func(&phdr[i], arg);
    }
    startup(argc-1,args+1,ehdr->e_entry);

   
    return 0;
}

void load_phdr(Elf32_Phdr *phdr, int fd) {
    Elf32_Phdr *current_phdr = phdr;
    off_t offset;
    void *addr;
    int prot;
   
    while (current_phdr->p_type != PT_NULL) {
        if (current_phdr->p_type == PT_LOAD) {
            prot = 0;

            if (current_phdr->p_flags & PF_R)
                prot |= PROT_READ;
           
            if (current_phdr->p_flags & PF_W)
                prot |= PROT_WRITE;
           
            if (current_phdr->p_flags & PF_X)
                prot |= PROT_EXEC;
           
            int mmap_flags = MAP_PRIVATE | MAP_FIXED;
            addr = current_phdr->p_vaddr&0xfffff000;
            offset = current_phdr->p_offset&0xfffff000;
            int padding = current_phdr->p_vaddr & 0xfff;

            //printf("%x %x %x %x %x %x",addr, current_phdr->p_memsz, prot, current_phdr->p_flags, fd);
            if (mmap(addr, current_phdr->p_memsz+padding, prot, mmap_flags, fd, offset) == MAP_FAILED) {
                perror("mmap failed");
                //return;
            }
           
            if (lseek(fd, offset, SEEK_SET) == -1) {
                perror("lseek failed");
                return;
            }
           
            if (read(fd, addr, current_phdr->p_filesz) == -1) {
                perror("read failed");
                return;
            }
        }
        current_phdr++;
    }
}

int main(int argc, char **argv){
    int fd;
    FILE* file;
    if (argc<2) perror("Wrong number of arguments");
    else {
        file=fopen(argv[1], "r+");
        if (file==NULL){
        perror("Failed to open file");
        return 1;
        }
        fd=fileno(file);
        if (fseek(file,0,SEEK_END)!=0){
        perror("Failed to move file pointer");
        return 1;
        }
        int length=ftell(file);
        if (length<0){
        perror("Invalid file size");
        return 1;
        }
        fseek(file, 0, SEEK_SET);
        void *map_start=mmap(NULL, length, PROT_READ | PROT_EXEC, MAP_PRIVATE, fd, 0);
        if (map_start == MAP_FAILED) {
        perror("Error mapping file");
        return 1;
    }
        foreach_phdr(map_start, load_phdr, fd, argv, argc);
        munmap(map_start, length);
        fclose(file);
        
    } 
    return 0;
}

