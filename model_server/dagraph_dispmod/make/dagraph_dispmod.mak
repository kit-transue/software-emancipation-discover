include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/dagraph_dispmod.lib

OBJS= \
	$(INTDIR)/DAGHandlerXREF.obj \
	$(INTDIR)/DGDataAccessXREF.obj \
	$(INTDIR)/DGNodeDataXREF.obj \
	$(INTDIR)/DGPathXREF.obj \
	$(INTDIR)/DGPrintXREF.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../dagraph/include \
/I ../../debug/include \
/I ../../dd/include \
/I ../../obj/include \
/I ../../project_pdf/include \
/I ../../search/include \
/I ../../xref/include \
/I ../../../gen/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \

include $(ADMMAKE)/targets.mak
