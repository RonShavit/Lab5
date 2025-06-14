my_loader: my_loader.c
	gcc -g -Wall -m32 -o my_loader my_loader.c

clean:
	rm my_loader