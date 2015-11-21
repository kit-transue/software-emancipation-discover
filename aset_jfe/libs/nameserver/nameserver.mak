PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/nameserver.lib

OBJS=$(INTDIR)/nameServCalls.obj

VPATH=$(PATHTOROOT)/nameserver/src;$(PATH2BS)/nameserver/src

CPP_INCLUDES=/I "$(PATHTOROOT)/nameserver/include" \
	/I "$(PATH2BS)/nameserver/include"

include $(MAKESCRIPTSPATH)/targets.mak

