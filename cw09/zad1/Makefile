CC=gcc
FLAGS=-g -Wall -lpthread -lm
MAIN=main
LIBDIR=libout
LIBS=$(LIBDIR)/utils.a

all: main 
	make clean

lib:
	mkdir -p $(LIBDIR) 
	$(CC) -c src/utils.c
	ar crs $(LIBDIR)/utils.a utils.o

main: lib
	$(CC) $(FLAGS) -o $(MAIN).out src/$(MAIN).c $(LIBS)

clean:
	rm *.o

