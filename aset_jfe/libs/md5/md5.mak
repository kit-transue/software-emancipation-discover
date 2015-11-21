PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/md5.lib

OBJS= \
	$(INTDIR)/md5c.obj 

VPATH=$(PATHTOROOT)/md5/src;$(PATH2BS)/md5/src

CPP_INCLUDES=\
	/I $(PATHTOROOT)/md5/include\
	/I $(PATH2BS)/md5/include\

include $(MAKESCRIPTSPATH)/targets.mak
