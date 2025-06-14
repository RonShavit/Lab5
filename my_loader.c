#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>



int main(int argc, char* argv[])
{
    int op = -1;
    Elf32_Ehdr* header = malloc(52);
    if (argc<2)
    {
        printf("Usage : ./my_loader <file_name>\n");
        exit(0);
    }
    op = open(argv[1],O_RDWR);
    if(op<0)
    {
        fprintf(stderr,"ERROR opening file %s",argv[1]);
        perror(" ");
        exit(0);
    }
    read(op,header,sizeof(*header));
    printf("%d\n",header->e_ehsize);

    close(op);
    free(header);

    
    
}