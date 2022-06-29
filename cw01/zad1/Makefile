LIB=libblock.c
OBJ = libblock.o
LIBSTATIC = libblock.a
LIBDYNAMIC = libblock.so
STATICOUT = static
DYNAMICOUT = dynamic
ENTRY = main.c
LIBDIR = .

all:
	make static
	make dynamic
	make clean

static:
	gcc -Wall -c $(LIB)
	ar crs $(LIBSTATIC) $(OBJ)
	gcc -Wall -o $(STATICOUT) $(ENTRY) -L$(LIBDIR) $(LIBSTATIC) 

dynamic:
	gcc -Wall -fPIC -c $(LIB)
	gcc -Wall -shared -o $(LIBDYNAMIC) $(OBJ)
	gcc -Wall -o $(DYNAMICOUT) $(ENTRY) -L. $(LIBDYNAMIC)

clean:
	rm $(OBJ)
