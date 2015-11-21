include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/dagraph_disgui.lib

OBJS= \
	$(INTDIR)/appDGraph.obj \
	$(INTDIR)/disUIHandler.obj \
	$(INTDIR)/ldrDGraph.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../dagraph/include \
/I ../../dagraph_dispmod/include \
/I ../../debug/include \
/I ../../dd/include \
/I ../../driver_aset/include \
/I ../../interface/include \
/I ../../ldr/include \
/I ../../obj/include \
/I ../../project_pdf/include \
/I ../../rtl/include \
/I ../../search/include \
/I ../../ste/include \
/I ../../ui/include \
/I ../../ui_OODT/include \
/I ../../ui_viewer/include \
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
