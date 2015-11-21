INCLUDES=-I../h
DEFINES=-DCONDITIONAL_MEMBER

OBJ=src1.o
CC=aCompiler
CFLAGS=$(INCLUDES) $(DEFINES)
EXE=exe

$(EXE) : $(OBJ)
	$(CC) $(OBJ) -o $@

clean:
	rm -f $(OBJ)
	rm -f $(EXE)

