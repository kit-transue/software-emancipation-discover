include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/IDE_editor.lib

OBJS= \
	$(INTDIR)/ide_editor.obj \
	$(INTDIR)/COM_trace.obj

VPATH=../src;\
      ../COM_trace

CPP_INCLUDES=\
/I ../include\
/I ../../IDE/include\
/I ../../IDE/Logger\
/I ../../../machine/include\
/I $(PATH2BS)/machine/include\

include $(ADMMAKE)/targets.mak
