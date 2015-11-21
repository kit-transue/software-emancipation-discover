PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/update.lib

OBJS= \
	$(INTDIR)/GAT_update.obj \
	$(INTDIR)/hash.obj \
	$(INTDIR)/memman.obj \
	$(INTDIR)/mrg_main.obj \
	$(INTDIR)/mrg_util.obj

VPATH=../../src

CPP_INCLUDES=\
	/I ../../include \
	/I $(PATHTOROOT)/gen/include \
	/I $(PATHTOROOT)/nihcl/include \
	/I $(PATHTOROOT)/machine/include \
	/I $(PATHTOROOT)/stream_message/include \
	/I $(PATH2BS)/gen/include \
	/I $(PATH2BS)/nihcl/include \
	/I $(PATH2BS)/machine/include \
	/I $(PATH2BS)/stream_message/include


CPP_PROJ=/MD\
 $(CPP_OPTS)\
 $(CPP_WINDOWS)\
 $(CPP_DEFINES)\
 $(CPP_INCLUDES)\
 $(CPP_FINISH)

include $(MAKESCRIPTSPATH)/targets.mak

