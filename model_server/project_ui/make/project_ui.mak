include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/project_ui.lib

OBJS= \
	$(INTDIR)/merge.obj \
	$(INTDIR)/projmerge.obj
        
VPATH=../src

CPP_INCLUDES=\
/I ../include \

include $(ADMMAKE)/targets.mak
