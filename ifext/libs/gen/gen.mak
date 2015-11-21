PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak
 
LIBRARY=$(INTDIR)/gen.lib

OBJS= \
	$(INTDIR)/psetmem.obj \
	$(INTDIR)/genString.obj

VPATH=$(PATHTOROOT)/gen/src;$(PATH2BS)/gen/src

CPP_INCLUDES=\
	/I $(PATHTOROOT)/model_server/ui/include \
	/I $(PATHTOROOT)/model_server/interface/include \
	/I $(PATHTOROOT)/gen/include \
	/I $(PATHTOROOT)/stream_message/include \
	/I $(PATHTOROOT)/machine/include \
	/I $(PATH2BS)/model_server/ui/include \
	/I $(PATH2BS)/model_server/interface/include \
	/I $(PATH2BS)/gen/include \
	/I $(PATH2BS)/stream_message/include \
	/I $(PATH2BS)/machine/include

CPP_PROJ=/MD\
 $(CPP_OPTS)\
 $(CPP_WINDOWS)\
 $(CPP_DEFINES)\
 $(CPP_INCLUDES)\
 $(CPP_FINISH)

include $(MAKESCRIPTSPATH)/targets.mak


