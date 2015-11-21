PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/mtk.lib

OBJS= \
	$(INTDIR)/mtkargs.obj \
	$(INTDIR)/mtkchangeitem.obj \
	$(INTDIR)/difffile.obj \
	$(INTDIR)/diffmanager.obj \
	$(INTDIR)/mtkdiffmanager.obj \
	$(INTDIR)/framework.obj \
	$(INTDIR)/mtkframework.obj \
	$(INTDIR)/memman.obj \
	$(INTDIR)/servercomm.obj \
	$(INTDIR)/mtktextbuffer.obj \
	$(INTDIR)/mtkmain.obj

VPATH=../../src

CPP_INCLUDES=\
	/I $(PATHTOROOT)/xincludes\
	/I $(PATHTOROOT)/mrg_ui/include\
	/I $(PATHTOROOT)/gen/include\
	/I $(PATHTOROOT)/dis_tcl8.3/generic\
	/I $(PATHTOROOT)/dis_tk8.3/generic\
	/I $(PATHTOROOT)/nihcl/include\
	/I $(PATHTOROOT)/machine/include\
	/I $(PATHTOROOT)/stream_message/include \
	/I $(PATH2BS)/xincludes\
	/I $(PATH2BS)/mrg_ui/include\
	/I $(PATH2BS)/gen/include\
	/I $(PATH2BS)/dis_tcl8.3/generic\
	/I $(PATH2BS)/dis_tk8.3/generic\
	/I $(PATH2BS)/nihcl/include\
	/I $(PATH2BS)/machine/include\
	/I $(PATH2BS)/stream_message/include

CPP_PROJ=/MD\
 $(CPP_OPTS)\
 $(CPP_WINDOWS)\
 $(CPP_DEFINES)\
 $(CPP_INCLUDES)\
 $(CPP_FINISH)

include $(MAKESCRIPTSPATH)/targets.mak
