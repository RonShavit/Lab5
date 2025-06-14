#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>

int checkMagicNumber(unsigned char number[])
{
    if (number[0] != 0x7f || number[1] != 'E' || number[2] != 'L' || number[3] != 'F' || number[4] != 1)
        return 0;
    return 1;
}

int main(int argc, char *argv[])
{
    int op = -1;
    Elf32_Ehdr *header = malloc(52);
    if (argc < 2)
    {
        printf("Usage : ./my_loader <file_name>\n");
        exit(0);
    }
    op = open(argv[1], O_RDWR);
    if (op < 0)
    {
        fprintf(stderr, "ERROR opening file %s", argv[1]);
        perror(" ");
        exit(0);
    }

    read(op, header, sizeof(*header));
    if (checkMagicNumber(header->e_ident) == 0)
    {
        fprintf(stderr, "ERROR reading file %s : Not an ELF32 File\n", argv[1]);
        close(op);
        free(header);
        exit(0);
    }

    printf("%s\n", header->e_ident);

    close(op);
    free(header);
}