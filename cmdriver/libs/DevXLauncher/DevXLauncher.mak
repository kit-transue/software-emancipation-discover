PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/DevXLauncher.lib

OBJS= \
	$(INTDIR)/startproc.obj

VPATH=$(PATHTOROOT)/DevXLauncher/src;$(PATH2BS)/DevXLauncher/src

CPP_INCLUDES=\
	/I $(PATHTOROOT)/DevXLauncher/include \
	/I $(PATH2BS)/DevXLauncher/include \
	/I $(PATHTOROOT)/nameserver/include \
	/I $(PATH2BS)/nameserver/include

CPP_PROJ=/MD \
 $(CPP_OPTS)\
 $(CPP_WINDOWS)\
 $(CPP_DEFINES)\
 $(CPP_INCLUDES)\
 $(CPP_FINISH)

include $(MAKESCRIPTSPATH)/targets.mak


