include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/lde.lib

OBJS= \
	$(INTDIR)/lde-browser-hierarchy.obj \
	$(INTDIR)/lde-decorate.obj \
	$(INTDIR)/lde-hierarchy.obj \
	$(INTDIR)/lde-RTL.obj \
	$(INTDIR)/lde_erd.obj \
	$(INTDIR)/lde_reference.obj \
	$(INTDIR)/lde_smod_short.obj \
	$(INTDIR)/lde_smt_class.obj \
	$(INTDIR)/lde_smt_flowchart.obj \
	$(INTDIR)/lde_smt_goto.obj \
	$(INTDIR)/lde_smt_hierarchy.obj \
	$(INTDIR)/lde_smt_loop.obj \
	$(INTDIR)/lde_smt_switch.obj \
	$(INTDIR)/lde_smt_text.obj \
	$(INTDIR)/lde_smt_tree.obj \
	$(INTDIR)/lde_ste_hierarchy.obj \
	$(INTDIR)/lde_subsys_map.obj \
	$(INTDIR)/lde_symbol.obj \
	$(INTDIR)/ldeList.obj \
	$(INTDIR)/ldeSmod.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../cmd/include \
/I ../../driver_aset/include \
/I ../../dd/include \
/I ../../gra/include \
/I ../../interface/include \
/I ../../ldr/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../project/include \
/I ../../project_pdf/include \
/I ../../rtl/include \
/I ../../save/include \
/I ../../smt/include \
/I ../../ste/include \
/I ../../ste_disstub/include \
/I ../../style/include \
/I ../../subsystem/include \
/I ../../ui/include \
/I ../../ui_OODT/include \
/I ../../view/include \
/I ../../view_rtl/include \
/I ../../xref/include \
/I ../../../gen/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../paracancel/include \
/I ../../../stream_message/include \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/paracancel/include \
/I $(PATH2BS)/stream_message/include \

include $(ADMMAKE)/targets.mak
