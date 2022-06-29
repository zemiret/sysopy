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
	@gcc -c $(LIB)
	@ar crs $(LIBSTATIC) $(OBJ)
	@gcc $(OPT) -o $(STATICOUT) $(ENTRY) -L$(LIBDIR) $(LIBSTATIC) 
	@echo "STATIC"
	./static 10 benchmark

shared:
	@gcc -fPIC -c $(LIB)
	@gcc -shared -o $(LIBSHARED) $(OBJ)
	@gcc $(OPT) -o $(SHAREDOUT) $(ENTRY) -L. $(LIBSHARED)
	@echo "SHARED"
	./shared 10 benchmark

dynamic:
	@gcc -fPIC -c $(LIB)
	@gcc -shared -o $(LIBSHARED) $(OBJ)
	@gcc $(OPT) -o $(DYNAMICOUT) $(DYNENTRY) -ldl 
	@echo "DYNAMIC"
	./dynamic 10 benchmark

clean:
	@rm $(OBJ)
