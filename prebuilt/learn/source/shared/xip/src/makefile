LIB_MRM = -lMrm
LIB_XM  = -lXm
LIB_XT  = -lXt
LIB_XMU = -lXmu
LIB_X11 = -lX11
LIB_MIX = $(PSETHOME)/lib/libmix.o
LIB_DT  = -L. -lDt

XIP_INCLUDE = ../include

COMPILE_FLAGS     = -g -pipe -DDEBUG -I$(XIP_INCLUDE)

LDFLAGS           = 
DYNAMIC_LDFLAGS   = 
DYNAMIC_LIBRARIES = -Bdynamic $(DYNAMIC_LDFLAGS) $(LIB_MIX)
STATIC_LDFLAGS    = -L/usr/lib -L/usr/lib/X11/.. -L/usr/openwin/lib 
STATIC_LIBRARIES  = -Bstatic $(STATIC_LDFLAGS) $(LIB_DT) \
		    $(LIB_MRM) $(LIB_XM) $(LIB_XMU) $(LIB_XT) $(LIB_X11) 

CC = cc

SRCS =	xip.c \
        canvas.c \
        pipeline.c \
        stage.c \
        image_win.c \
	color.c \
	fonts.c \
	init.c

OBJS =  xip.o \
        canvas.o \
        pipeline.o \
        stage.o \
        image_win.o

LIB_OBJS = color.o \
	   fonts.o \
	   init.o

UIDS = xip.uid

EXEC = xip

executable: $(EXEC)

$(EXEC): $(OBJS) libDt.a $(UIDS)
	$(CC) -u _XCreateSimpleWindow  $(LDFLAGS) -o $(EXEC).tmp $(OBJS) $(STATIC_LIBRARIES) $(DYNAMIC_LIBRARIES)
	mv $(EXEC).tmp $(EXEC)
	rm -f $(EXEC).pag $(EXEC).dir
	symtab $(EXEC)


libDt.a: $(LIB_OBJS)
	ar clq libDt.a $(LIB_OBJS)
	ranlib libDt.a 


all: $(OBJS) $(EXEC)

.SUFFIXES: .o .c .uil .uid

.c.o: 
	$(CC) -c $(COMPILE_FLAGS) $<

.uil.uid:  
	uil -o $(EXEC).tmp.uid $(EXEC).uil 
	mv $(EXEC).tmp.uid $(EXEC).uid

clean:
	\rm -f xip xip.dir xip.pag *.o






