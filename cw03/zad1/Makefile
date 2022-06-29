lib:
	gcc -c utils/utils.c 
	ar crs utils.a utils.o

all: lib
	gcc -g -o out -Wall -std=c99 -D_DEFAULT_SOURCE main.c utils.a
	make clean

clean:
	rm -f *.o
