include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/machine.lib

OBJS= \
	$(INTDIR)/DisFName.obj \
	$(INTDIR)/DisRegistry.obj \
	$(INTDIR)/alloca.obj \
	$(INTDIR)/cLibraryFunctions.obj \
	$(INTDIR)/fd_fstream.obj \
	$(INTDIR)/fd_streambuf.obj \
	$(INTDIR)/machdep.obj \
	$(INTDIR)/map_manager.obj \
	$(INTDIR)/shell_calls.obj \
	$(INTDIR)/test.obj

VPATH=../../../machine/src; \
      $(PATH2BS)/machine/src

CPP_INCLUDES=\
/I ../../IDE/include \
/I ../../../gen/include \
/I ../../../machine/include \
/I ../../../stream_message/include \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/stream_message/include \

include $(ADMMAKE)/targets.mak
