PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/src.lib

OBJS=\
	$(INTDIR)/dish.obj \
	$(INTDIR)/SocketComm.obj

VPATH=../../src

CPP_INCLUDES=\
	/I ../../include \
	/I $(PATHTOROOT)/nameserver/include \
	/I $(PATHTOROOT)/xxinterface/include \
	/I $(PATHTOROOT)/gen/include \
	/I $(PATH2BS)/nameserver/include \
	/I $(PATH2BS)/xxinterface/include \
	/I $(PATH2BS)/gen/include

include $(MAKESCRIPTSPATH)/targets.mak

