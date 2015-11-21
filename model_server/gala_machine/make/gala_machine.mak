include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/gala_machine.lib

OBJS= \
        $(INTDIR)/alloca.obj \
        $(INTDIR)/gala_cLibraryFunctions.obj \
        $(INTDIR)/DisFName.obj \
        $(INTDIR)/DisRegistry.obj \
        $(INTDIR)/fd_fstream.obj \
        $(INTDIR)/fd_streambuf.obj \
        $(INTDIR)/machdep.obj \
        $(INTDIR)/map_manager.obj \
        $(INTDIR)/nt_gala_printf.obj \
        $(INTDIR)/shell_calls.obj \
        $(INTDIR)/test.obj

VPATH=../src;\
      ../../../machine/src;\
      $(PATH2BS)/machine/src
      
CPP_INCLUDES=\
/I ../include \
/I ../../IDE/include \
/I ../../dd/include \
/I ../../obj/include \
/I ../../rtl/include \
/I ../../ui_SEARCH/include \
/I ../../xref/include \
/I ../../../gen/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../stream_message/include \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/stream_message/include \

include $(ADMMAKE)/targets.mak
