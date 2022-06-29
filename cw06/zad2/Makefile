CC=gcc
FLAGS=-g -Wall -lrt
SERVER=server
CLIENT=client
LIBDIR=libout
LIBS=$(LIBDIR)/utils.a $(LIBDIR)/communication.a

lib:
	mkdir -p $(LIBDIR) 
	$(CC) -c src/utils.c
	$(CC) -c src/communication.c
	ar crs $(LIBDIR)/utils.a utils.o
	ar crs $(LIBDIR)/communication.a communication.o

client: lib
	$(CC) $(FLAGS) -o $(CLIENT).out src/$(CLIENT).c $(LIBS)

server: lib
	$(CC) $(FLAGS) -o $(SERVER).out src/$(SERVER).c $(LIBS)

all: client server 
	make clean

clean:
	rm *.o

