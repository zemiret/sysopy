CC=gcc
FLAGS=-Wall -g

all:
	make clock 
	make date
	make clean 

clock:
	$(CC) $(FLAGS) -o clock.out clock.c

date:
	$(CC) $(FLAGS) -o date.out date.c

clean:
	rm -f *.o

