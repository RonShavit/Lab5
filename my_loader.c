#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <sys/mman.h>
#include <errno.h>
extern int system_call();
extern int startup();

char *title_format = "TYPE    Offset   VirtAdrr   PhysAdrr   Filesiz   Memsiz  FLG Align\n";
char *format = "%-7s %#08x %#010x %#010x %#09x %#07x %1c%1c%1c %#06x\n";

int checkMagicNumber(unsigned char number[])
{
    if (number[0] != 0x7f || number[1] != 'E' || number[2] != 'L' || number[3] != 'F' || number[4] != 1) // check if file is ELF32 file
        return 0;
    return 1;
}

void test_print(Elf32_Phdr *phdr, int i)
{
    printf("prog header %d at add %X", i, phdr->p_vaddr);
}

void readelf_L(Elf32_Phdr *phdr, int j)
{
    char *type;
    char flag[3];
    if (j == 0)
    {
        printf("%s", title_format);
    }
    switch (phdr->p_type)
    {
    case PT_NULL:
        type = "NULL";
        break;
    case PT_LOAD:
        type = "LOAD";
        break;
    case PT_DYNAMIC:
        type = "DYNAMIC";
        break;
    case PT_INTERP:
        type = "INTERP";
        break;
    case PT_NOTE:
        type = "NOTE";
        break;
    case PT_SHLIB:
        type = "SHLIB";
        break;
    case PT_PHDR:
        type = "PHDR";
        break;

    default:
        type = "ELSE";
        break;
    }

    for (int i = 0; i < 3; i++)
    {
        flag[i] = ' ';
    }
    if (phdr->p_flags >= 0b100)
    {
        flag[0] = 'R';
    }
    if (phdr->p_flags >= 0b10 && (phdr->p_flags < 0b100 || phdr->p_flags - 0b100 >= 0b10))
    {
        flag[1] = 'W';
    }
    if (phdr->p_flags % 2 == 1)
    {
        flag[2] = 'E';
    }

    printf(format, type, phdr->p_offset, phdr->p_vaddr, phdr->p_paddr, phdr->p_filesz, phdr->p_memsz, flag[0], flag[1], flag[2], phdr->p_align);
    if (phdr->p_type != PT_DYNAMIC && phdr->p_type != PT_INTERP)
    {
        printf("protection flags: ");
        if (flag[0] == 'R')
        {
            printf("PROT_READ ");
            if (flag[1] == 'W' || flag[2] == 'E')
                printf("| ");
        }
        if (flag[1] == 'W')
        {
            printf("PROT_WRITE ");
            if (flag[2] == 'E')

                printf("| ");
        }
        if (flag[2] == 'E')
            printf("PROT_EXEC");
        if (flag[1] == ' ' && flag[2] == ' ' && flag[3] == ' ')
        {
            printf("PROT_NONE");
        }
        printf("\n");
        printf("mapping flags: MAP_PRIVATE | MAP_FIXED\n");
    }
}

void load_phdr(Elf32_Phdr *phdr, int fd)
{

    if (phdr->p_type == PT_LOAD)
    {
        unsigned int vaddr = phdr->p_vaddr & 0xfffff000;
        unsigned int offset = phdr->p_offset & 0xfffff000;
        unsigned int padding = phdr->p_vaddr & 0xfff;

        int flags = 0;
        if (phdr->p_flags & 0x1)
            flags |= PROT_EXEC;
        if (phdr->p_flags & 0x2)
            flags |= PROT_WRITE;
        if (phdr->p_flags & 0x4)
            flags |= PROT_READ;

        printf("%s", title_format);
        readelf_L(phdr, 1);

        printf("vaddr      offset     padding    flg fd\n");
        printf("%#010x %#010x %#010x %1d%1d%1d %d\n", vaddr, offset, padding, flags / 0b100, (flags % 0b100) / 0b10, flags % 0b10, fd);

        void *map = mmap((void *)vaddr, phdr->p_memsz + padding, flags, MAP_PRIVATE | MAP_FIXED , fd, offset);

        if (errno != 0)
        {
            perror("ERROR mapping memory");
            //printf("ERRcode %d\n", errno);
        }
        else
        {
            printf("Mapped memoty successfully!\n");
        }
    }
}

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg)
{
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map_start;
    if (checkMagicNumber(ehdr->e_ident))
    {

        Elf32_Phdr *phdr_table = (Elf32_Phdr *)((char *)map_start + ehdr->e_phoff);

        for (int i = 0; i < ehdr->e_phnum; i++)
        {
            func(&phdr_table[i], arg);
        }
    }
}

int main(int argc, char *argv[])
{
    int op = -1;

    Elf32_Ehdr *header = NULL;

    if (argc < 2) // No file name given
    {
        printf("Usage : ./my_loader <file_name>\n");
        exit(0);
    }
    op = open(argv[1], O_RDWR | O_EXCL);
    if (op < 0) // cannot open file
    {
        fprintf(stderr, "ERROR opening file %s", argv[1]);
        perror(" ");
        exit(0);
    }

    lseek(op, 0, SEEK_SET);
    int size = lseek(op, 0, SEEK_END);
    lseek(op, 0, SEEK_SET);
    header = malloc(52);
    read(op, header, sizeof(*header));
    if (checkMagicNumber(header->e_ident) == 0) // file is not ELF32
    {
        fprintf(stderr, "ERROR reading file %s : Not an ELF32 File\n", argv[1]);
        close(op);
        free(header);
        exit(0);
    }

    void *map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, op, 0);

    foreach_phdr(map, load_phdr, op);
    startup(argc,argv,header->e_entry);

    close(op);
    free(header);
}