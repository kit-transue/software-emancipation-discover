PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/machine.lib

OBJS= \
	$(INTDIR)/machdep.obj \
	$(INTDIR)/DisFName.obj \
	$(INTDIR)/DisRegistry.obj \
	$(INTDIR)/cLibraryFunctions.obj

VPATH=$(PATHTOROOT)/machine/src;$(PATH2BS)/machine/src

CPP_INCLUDES=\
	/I $(PATHTOROOT)/machine/include \
	/I $(PATHTOROOT)/gen/include \
	/I $(PATHTOROOT)/stream_message/include \
	/I $(PATH2BS)/machine/include \
	/I $(PATH2BS)/gen/include \
	/I $(PATH2BS)/stream_message/include

include $(MAKESCRIPTSPATH)/targets.mak
