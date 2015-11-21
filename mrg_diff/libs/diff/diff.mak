PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/diff.lib

OBJS= \
	$(INTDIR)/main.obj \
	$(INTDIR)/mrg_diff.obj \
	$(INTDIR)/mrgFile.obj \
	$(INTDIR)/tokenize.obj \
	$(INTDIR)/stubs.obj

VPATH=../../src

CPP_INCLUDES=\
	/I $(PATHTOROOT)/mrg_diff/include \
	/I $(PATHTOROOT)/gen/include \
	/I $(PATHTOROOT)/nihcl/include \
	/I $(PATHTOROOT)/machine/include \
	/I $(PATH2BS)/mrg_diff/include \
	/I $(PATH2BS)/gen/include \
	/I $(PATH2BS)/nihcl/include \
	/I $(PATH2BS)/machine/include

CPP_PROJ=/MD\
 $(CPP_OPTS)\
 $(CPP_WINDOWS)\
 $(CPP_DEFINES)\
 $(CPP_INCLUDES)\
 $(CPP_FINISH)

include $(MAKESCRIPTSPATH)/targets.mak
