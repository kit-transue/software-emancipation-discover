include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/dagraph.lib

OBJS= \
	$(INTDIR)/DAGHandler.obj \
	$(INTDIR)/DAGraph.obj \
	$(INTDIR)/DGDataAccess.obj \
	$(INTDIR)/DGNode.obj \
	$(INTDIR)/DGNodeData.obj \
	$(INTDIR)/DGPathFilter.obj \
	$(INTDIR)/DGPathHandler.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../debug/include \
/I ../../search/include \

include $(ADMMAKE)/targets.mak
