PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/gen.lib

OBJS= \
	$(INTDIR)/charstream.obj \
	$(INTDIR)/genString.obj \
	$(INTDIR)/genStringPlus.obj \
	$(INTDIR)/genTmpfile.obj \
	$(INTDIR)/globals.obj \
	$(INTDIR)/vpopen.obj

VPATH=$(PATHTOROOT)/gen/src;$(PATH2BS)/gen/src

CPP_INCLUDES=\
	/I $(PATHTOROOT)/model_server/process/include\
	/I $(PATHTOROOT)/machine/include\
	/I $(PATHTOROOT)/stream_message/include\
	/I $(PATHTOROOT)/gen/include\
	/I $(PATH2BS)/model_server/process/include\
	/I $(PATH2BS)/machine/include\
	/I $(PATH2BS)/stream_message/include\
	/I $(PATH2BS)/gen/include

CPP_PROJ=/MD\
 $(CPP_OPTS)\
 $(CPP_WINDOWS)\
 $(CPP_DEFINES)\
 $(CPP_INCLUDES)\
 $(CPP_FINISH)

include $(MAKESCRIPTSPATH)/targets.mak

