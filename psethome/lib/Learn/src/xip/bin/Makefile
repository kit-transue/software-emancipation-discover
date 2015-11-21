CC	= gcc
LDFLAGS	= 

CFLAGS	= 
LIBS	= 

SRCDIR	= ../src
INCLUDES=-I../../../private/xip/include -I../../../private/sysinclude \
	-I../../../src/xip/include -I../../../src/sysinclude

OBJ = \
	 xip.o \
	canvas.o \
	pipeline.o \
	stage.o \
	image_win.o \
	color.o \
	fonts.o \
	init.o

.IGNORE:
.DEFAULT:
	@if [ -f $(SRCDIR)/$(@:.o=.c) ] ;\
	then \
	  $(MAKE) OBJ=$(@) $(@) ;\
	else \
	  echo "make: Fatal error: Don't know how to make target \`$(@)'" ;\
	fi

.c.o:	$(CC) $(CFLAGS) $(INCLUDES) -c $<

executable: $(OBJ)
	$(CC) $(CFLAGS) -o executable $(OBJ) $(LDFLAGS) $(LIBS)
	@rm -f *.dir *.pag
	`discover -where`/bin/symtab $@

$(OBJ): $(SRCDIR)/$$(@:.o=.c)
	$(CC) $(CFLAGS) $(INCLUDES) -c $(SRCDIR)/$(@:.o=.c) -o $@

makemf:
	@echo "Dummy Rule: .DEFAULT will accept new src files"

clean:
	@rm -f core
	@rm -f executable
	@rm -f executable.pag executable.dir
	@rm -f $(OBJ)
	@rm -f junk*
