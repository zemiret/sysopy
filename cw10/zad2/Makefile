CC=gcc
FLAGS=-g -Wall -lpthread -lm
SERVER=server
CLIENT=client
LIBDIR=libout
LIBS=$(LIBDIR)/utils.a $(LIBDIR)/connection.a

all: lib 
	$(CC) $(FLAGS) -o $(SERVER).out src/$(SERVER).c $(LIBS)
	$(CC) $(FLAGS) -o $(CLIENT).out src/$(CLIENT).c $(LIBS)
	make clean

lib:
	mkdir -p $(LIBDIR) 
	$(CC) -c src/utils.c
	$(CC) -c src/connection.c
	ar crs $(LIBDIR)/utils.a utils.o
	ar crs $(LIBDIR)/connection.a connection.o

main: lib
	$(CC) $(FLAGS) -o $(MAIN).out src/$(MAIN).c $(LIBS)

clean:
	rm *.o

