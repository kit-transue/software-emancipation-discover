PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/gdchart.lib

OBJS= \
	$(INTDIR)/gdchart.obj \
	$(INTDIR)/price_conv.obj

VPATH=../../gdchart/src

CPP_INCLUDES=\
	/I ../../gdchart/include \
	/I ../../gd/include

CPP_PROJ=/MT\
 $(CPP_OPTS)\
 $(CPP_WINDOWS)\
 $(CPP_DEFINES)\
 $(CPP_INCLUDES)\
 $(CPP_FINISH)

include $(MAKESCRIPTSPATH)/targets.mak

