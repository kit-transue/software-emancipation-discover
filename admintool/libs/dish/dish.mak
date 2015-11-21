PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/dish.lib

OBJS=\
	$(INTDIR)/SocketComm.obj

VPATH=$(PATHTOROOT)/dish/src;$(PATH2BS)/dish/src

CPP_INCLUDES=\
	/I $(PATHTOROOT)/dish/include \
	/I $(PATH2BS)/dish/include

CPP_PROJ=/ML \
 $(CPP_OPTS)\
 $(CPP_WINDOWS)\
 $(CPP_DEFINES)\
 $(CPP_INCLUDES)\
 $(CPP_FINISH)

include $(MAKESCRIPTSPATH)/targets.mak


