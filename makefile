my_loader: my_loader.c
	gcc -m32 -c my_loader.c -o my_loader.o
	nasm -f elf32 start.s -o start.o
	nasm -f elf32 startup.s -o startup.o
	ld -o my_loader my_loader.o startup.o start.o -L/usr/lib32 -lc -T linking_script -dynamic-linker /lib32/ld-linux.so.2



clean:
	rm my_loader