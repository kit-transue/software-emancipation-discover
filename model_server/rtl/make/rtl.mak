include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/rtl.lib

OBJS= \
	$(INTDIR)/RTL.obj \
	$(INTDIR)/RTL.h.obj \
	$(INTDIR)/RTL_interface.obj \
	$(INTDIR)/RTL_notify.obj \
	$(INTDIR)/RelationMonitor.h.obj \
	$(INTDIR)/objRTL.h.obj \
	$(INTDIR)/symbolTreeHeader.h.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../dd/include \
/I ../../driver_aset/include \
/I ../../ldr/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../project/include \
/I ../../project_pdf/include \
/I ../../ui/include \
/I ../../view/include \
/I ../../view_rtl/include \
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
