PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/editorinterface.lib

OBJS= \
	$(INTDIR)/EditorInterface.obj 

VPATH=$(PATHTOROOT)/editorinterface/src;$(PATH2BS)/editorinterface/src

CPP_INCLUDES=\
	/I $(PATHTOROOT)/editorinterface/include \
	/I $(PATH2BS)/editorinterface/include \
	/I $(PATHTOROOT)/nameserver/include \
	/I $(PATH2BS)/nameserver/include

CPP_PROJ=/MD \
 $(CPP_OPTS)\
 $(CPP_WINDOWS)\
 $(CPP_DEFINES)\
 $(CPP_INCLUDES)\
 $(CPP_FINISH)

include $(MAKESCRIPTSPATH)/targets.mak


