include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/ste_disstub.lib

OBJS= \
	$(INTDIR)/disstub.obj \
	$(INTDIR)/ste_communication.obj \
	$(INTDIR)/ste_init.obj \
	$(INTDIR)/steEditor.obj \
	$(INTDIR)/steHeader.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../dd/include \
/I ../../obj/include \
/I ../../ste/include \
/I ../../style/include \
/I ../../../gen/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../stream_message/include \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/stream_message/include \

include $(ADMMAKE)/targets.mak
