PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/src.lib

OBJS= \
	$(INTDIR)/checklist.obj \
	$(INTDIR)/psetmem.obj

VPATH=../../src

CPLUSPLUS_FLAGS=

CPP_INCLUDES=\
	/I $(PATHTOROOT)/nihcl/include\
	/I $(PATHTOROOT)/gen/include \
	/I $(PATH2BS)/nihcl/include\
	/I $(PATH2BS)/gen/include

CPP_PROJ=/MT \
 $(CPP_OPTS)\
 $(CPP_WINDOWS)\
 $(CPP_DEFINES)\
 $(CPP_INCLUDES)\
 $(CPP_FINISH)

include $(MAKESCRIPTSPATH)/targets.mak


