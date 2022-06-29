CC=gcc
FLAGS=-g -Wall
TRUCKER=trucker
LOADER=loader
LOADER_EXECUTOR=loader_executor
LIBDIR=libout
LIBS=$(LIBDIR)/utils.a $(LIBDIR)/transmission.a $(LIBDIR)/environmental.a

lib:
	mkdir -p $(LIBDIR) 
	$(CC) -c src/utils.c
	$(CC) -c src/transmission.c
	$(CC) -c src/environmental.c
	ar crs $(LIBDIR)/utils.a utils.o
	ar crs $(LIBDIR)/transmission.a transmission.o
	ar crs $(LIBDIR)/environmental.a environmental.o

loader: lib
	$(CC) $(FLAGS) -o $(LOADER).out src/$(LOADER).c $(LIBS)
	$(CC) $(FLAGS) -o $(LOADER_EXECUTOR).out src/$(LOADER_EXECUTOR).c $(LIBS)

trucker: lib
	$(CC) $(FLAGS) -o $(TRUCKER).out src/$(TRUCKER).c $(LIBS)

all: loader trucker
	make clean

clean:
	rm *.o

