CC=gcc
FLAGS=-g -Wall
LIB=utils.a
PROG=main

lib:
	gcc -c utils/utils.c
	ar crs $(LIB) utils.o

all: lib
	$(CC) $(FLAGS) -o $(PROG).out $(PROG).c $(LIB) 
	make clean

clean:
	rm *.o

