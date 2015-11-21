PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/scandsp.lib

OBJS= \
	$(INTDIR)/registry_if.obj

VPATH=$(PATHTOROOT)/scandsp/src;$(PATH2BS)/scandsp/src

CPP_INCLUDES=\
	/I $(PATHTOROOT)/scandsp/include \
	/I $(PATH2BS)/scandsp/include

CPP_PROJ=/MD \
 $(CPP_OPTS)\
 $(CPP_WINDOWS)\
 $(CPP_DEFINES)\
 $(CPP_INCLUDES)\
 $(CPP_FINISH)

include $(MAKESCRIPTSPATH)/targets.mak

