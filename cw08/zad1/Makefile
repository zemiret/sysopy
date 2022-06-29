CC=gcc
FLAGS=-g -Wall -lpthread -lm
MAIN=main
LIBDIR=libout
LIBS=$(LIBDIR)/utils.a $(LIBDIR)/matrix.a $(LIBDIR)/pgm.a 

lib:
	mkdir -p $(LIBDIR) 
	$(CC) -c src/utils.c
	$(CC) -c src/matrix.c
	$(CC) -c src/pgm.c
	ar crs $(LIBDIR)/utils.a utils.o
	ar crs $(LIBDIR)/matrix.a matrix.o
	ar crs $(LIBDIR)/pgm.a pgm.o

main: lib
	$(CC) $(FLAGS) -o $(MAIN).out src/$(MAIN).c $(LIBS)

all: main 
	make clean

clean:
	rm *.o

