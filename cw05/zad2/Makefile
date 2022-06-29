CC=gcc
FLAGS=-g -Wall
LIB=utils.a
MASTER=master
SLAVE=slave

lib:
	gcc -c utils/utils.c
	ar crs $(LIB) utils.o

all:
	make master
	make slave
	make clean

master: lib
	$(CC) $(FLAGS) -o $(MASTER).out $(MASTER).c $(LIB) 

slave: lib
	$(CC) $(FLAGS) -o $(SLAVE).out $(SLAVE).c $(LIB) 

clean:
	rm *.o

