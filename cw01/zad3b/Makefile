OPT = -O0

LIB=libblock.c
OBJ = libblock.o
LIBSTATIC = libblock.a
LIBSHARED = libblock.so

STATICOUT = static
SHAREDOUT = shared
DYNAMICOUT = dynamic

ENTRY = main.c
DYNENTRY = dmain.c
LIBDIR = .

all:
	make static
	make shared 
	make dynamic
	make clean

static:
	@gcc -Wall -c $(LIB)
	@ar crs $(LIBSTATIC) $(OBJ)
	@gcc -Wall $(OPT) -o $(STATICOUT) $(ENTRY) -L$(LIBDIR) $(LIBSTATIC) 
	@echo "STATIC"
	./static 10 benchmark

shared:
	@gcc -Wall -fPIC -c $(LIB)
	@gcc -Wall -shared -o $(LIBSHARED) $(OBJ)
	@gcc -Wall $(OPT) -o $(SHAREDOUT) $(ENTRY) -L. $(LIBSHARED)
	@echo "SHARED"
	LD_LIBRARY_PATH=. ./shared 10 benchmark

dynamic:
	@gcc -Wall -fPIC -c $(LIB)
	@gcc -Wall -shared -o $(LIBSHARED) $(OBJ)
	@gcc -Wall $(OPT) -o $(DYNAMICOUT) $(DYNENTRY) -ldl 
	@echo "DYNAMIC"
	LD_LIBRARY_PATH=. ./dynamic 10 benchmark

clean:
	@rm $(OBJ)
