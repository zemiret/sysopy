SENDER=sender
CATCHER=catcher
FLAGS=-g -Wall
LIBS=utils.a sigcommon.a

lib:
	gcc -c src/utils.c 
	ar crs utils.a utils.o
	gcc -c src/sigcommon.c
	ar crs sigcommon.a sigcommon.o

sender: lib
	gcc $(FLAGS) -o $(SENDER).out src/$(SENDER).c $(LIBS) 

catcher: lib
	gcc $(FLAGS) -o $(CATCHER).out src/$(CATCHER).c $(LIBS) 

all: 
	make sender
	make catcher
	make clean

clean:
	rm -f *.o

