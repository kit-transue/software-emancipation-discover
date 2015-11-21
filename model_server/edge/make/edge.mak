include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/edge.lib

OBJS= \
	$(INTDIR)/aset_graph.obj \
	$(INTDIR)/aset_node.obj \
	$(INTDIR)/constr_topsort.obj \
	$(INTDIR)/edge.obj \
	$(INTDIR)/interface.obj \
	$(INTDIR)/matrix.obj \
	$(INTDIR)/node.obj \
	$(INTDIR)/set_constraints.obj \
	$(INTDIR)/sugilevel.obj \
	$(INTDIR)/sugitopsort.obj \
	$(INTDIR)/treelayout.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../dd/include \
/I ../../driver_aset/include \
/I ../../gedge/include \
/I ../../gra/include \
/I ../../graGala/include \
/I ../../ldr/include \
/I ../../obj/include \
/I ../../ste/include \
/I ../../style/include \
/I ../../ui/include \
/I ../../ui_OODT/include \
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
