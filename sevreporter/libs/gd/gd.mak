PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/gd.lib

OBJS= \
	$(INTDIR)/gd.obj \
	$(INTDIR)/gdfontg.obj \
	$(INTDIR)/gdfontl.obj \
	$(INTDIR)/gdfontmb.obj \
	$(INTDIR)/gdfonts.obj \
	$(INTDIR)/gdfontt.obj

VPATH=../../gd/src

CPP_INCLUDES=\
	/I ../../gd/include

CPP_PROJ=/MT\
 $(CPP_OPTS)\
 $(CPP_WINDOWS)\
 $(CPP_DEFINES)\
 $(CPP_INCLUDES)\
 $(CPP_FINISH)

include $(MAKESCRIPTSPATH)/targets.mak

