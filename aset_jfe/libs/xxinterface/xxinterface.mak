PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/xxinterface.lib

OBJS= \
	$(INTDIR)/xxinterface.obj \
	$(INTDIR)/license_stubs.obj \
	$(INTDIR)/xxsn.obj

VPATH=$(PATHTOROOT)/xxinterface/src;$(PATH2BS)/xxinterface/src

CPP_INCLUDES=\
	/I $(PATHTOROOT)/model_server/ui/include\
	/I $(PATHTOROOT)/model_server/interface/include\
	/I $(PATHTOROOT)/FLEXlm/flexlm/current/machind\
	/I $(PATHTOROOT)/gen/include\
	/I $(PATHTOROOT)/stream_message/include\
	/I $(PATHTOROOT)/xxinterface/include\
	/I $(PATH2BS)/model_server/ui/include\
	/I $(PATH2BS)/model_server/interface/include\
	/I $(PATH2BS)/FLEXlm/flexlm/current/machind\
	/I $(PATH2BS)/gen/include\
	/I $(PATH2BS)/stream_message/include\
	/I $(PATH2BS)/xxinterface/include

include $(MAKESCRIPTSPATH)/targets.mak
