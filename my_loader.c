#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <elf.h>



int main(int argv, char* argc[])
{
    int op = -1;
    if (argv<2)
    {
        printf("Usage : ./my_loader <file_name>\n");
        exit(0);
    }
    op = open(argc[1],O_RDWR);
    if(op<0)
    {
        fprintf(stderr,"ERROR opening file %s",argc[1]);
        perror(" ");
    }
    
}