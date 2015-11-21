include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/gedge.lib

OBJS= \
	$(INTDIR)/_copyedge.obj \
	$(INTDIR)/_copygraph.obj \
	$(INTDIR)/_defaults.obj \
	$(INTDIR)/addcross.obj \
	$(INTDIR)/adttree.obj \
	$(INTDIR)/adtugraph.obj \
	$(INTDIR)/bico.obj \
	$(INTDIR)/constr_3D.obj \
	$(INTDIR)/constr_box.obj \
	$(INTDIR)/constr_desc.obj \
	$(INTDIR)/constr_finetuning.obj \
	$(INTDIR)/constr_layout.obj \
	$(INTDIR)/constr_manager.obj \
	$(INTDIR)/constr_net.obj \
	$(INTDIR)/constr_node.obj \
	$(INTDIR)/constr_order.obj \
	$(INTDIR)/constr_queue.obj \
	$(INTDIR)/constr_stab.obj \
	$(INTDIR)/defaults.obj \
	$(INTDIR)/dummy.obj \
	$(INTDIR)/graph.obj \
	$(INTDIR)/hopcroft.obj \
	$(INTDIR)/intersect.obj \
	$(INTDIR)/isi.obj \
	$(INTDIR)/planar.obj \
	$(INTDIR)/planarlayout.obj \
	$(INTDIR)/queue.obj \
	$(INTDIR)/set.obj \
	$(INTDIR)/slist.obj \
	$(INTDIR)/stnum.obj \
	$(INTDIR)/sugilayout.obj \
	$(INTDIR)/write.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../edge/include \
/I ../../../gen/include \
/I ../../../machine/include \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/machine/include \

include $(ADMMAKE)/targets.mak
