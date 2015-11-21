PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/pdf.lib

OBJS= \
	$(INTDIR)/path_simplify.obj

VPATH=$(PATHTOROOT)/model_server/project_pdf/src;$(PATH2BS)/model_server/project_pdf/src

CPP_INCLUDES=\
	/I $(PATHTOROOT)/model_server/project_pdf/include \
	/I $(PATH2BS)/model_server/project_pdf/include

CPP_PROJ=/MD \
 $(CPP_OPTS)\
 $(CPP_WINDOWS)\
 $(CPP_DEFINES)\
 $(CPP_INCLUDES)\
 $(CPP_FINISH)

include $(MAKESCRIPTSPATH)/targets.mak


