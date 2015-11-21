INCLUDES=-I../h
DEFINES=-DCONDITIONAL_MEMBER

OBJ=src1.o
CC=aCompiler
CFLAGS=$(INCLUDES) $(DEFINES)
EXE=exe

$(EXE) : $(OBJ)
	$(CC) $(OBJ) -o $@

src1.o : ../src1.c
	$(CC) -c $(CFLAGS) $?

clean:
	rm -f $(OBJ)
	rm -f $(EXE)

