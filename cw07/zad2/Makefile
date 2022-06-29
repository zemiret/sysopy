CC=gcc
FLAGS=-g -Wall -lpthread -lrt
TRUCKER=trucker
LOADER=loader
LOADER_EXECUTOR=loader_executor
LIBDIR=libout
LIBS=$(LIBDIR)/utils.a $(LIBDIR)/transmission.a

lib:
	mkdir -p $(LIBDIR) 
	$(CC) -c src/utils.c
	$(CC) -c src/transmission.c
	ar crs $(LIBDIR)/utils.a utils.o
	ar crs $(LIBDIR)/transmission.a transmission.o

loader: lib
	$(CC) $(FLAGS) -o $(LOADER).out src/$(LOADER).c $(LIBS)
	$(CC) $(FLAGS) -o $(LOADER_EXECUTOR).out src/$(LOADER_EXECUTOR).c $(LIBS)

trucker: lib
	$(CC) $(FLAGS) -o $(TRUCKER).out src/$(TRUCKER).c $(LIBS)

all: loader trucker
	make clean

clean:
	rm *.o

