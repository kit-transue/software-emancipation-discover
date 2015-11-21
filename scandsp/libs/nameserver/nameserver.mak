PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/nameserver.lib

OBJS= \
	$(INTDIR)/nameServCalls.obj

VPATH=$(PATHTOROOT)/nameserver/src;$(PATH2BS)/nameserver/src

CPP_INCLUDES=\
	/I $(PATHTOROOT)/nameserver/include \
	/I $(PATH2BS)/nameserver/include

CPP_PROJ=/MD \
 $(CPP_OPTS)\
 $(CPP_WINDOWS)\
 $(CPP_DEFINES)\
 $(CPP_INCLUDES)\
 $(CPP_FINISH)

include $(MAKESCRIPTSPATH)/targets.mak


