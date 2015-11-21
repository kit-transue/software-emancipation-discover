include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/DIS_pset.lib

OBJS= \
	$(INTDIR)/stubs.obj \
	$(INTDIR)/graViewLayer.obj \
	$(INTDIR)/getSelection.obj \
	$(INTDIR)/open_view.obj \
	$(INTDIR)/gviewerShell.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../galaxy_includes/include \
/I ../../gala/include \
/I ../../gala_communications/include \
/I ../../dd/include \
/I ../../driver_aset/include \
/I ../../gra/include \
/I ../../graGala/include \
/I ../../ldr/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../project/include \
/I ../../project_pdf/include \
/I ../../rtl/include \
/I ../../smt/include \
/I ../../ste/include \
/I ../../style/include \
/I ../../ui/include \
/I ../../ui_OODT/include \
/I ../../ui_misc/include \
/I ../../view/include \
/I ../../view_rtl/include \
/I ../../xref/include \
/I ../../../dis_tcl8.3/generic \
/I ../../../dis_tk8.3/generic \
/I ../../../gen/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I $(PATH2BS)/dis_tcl8.3/generic \
/I $(PATH2BS)/dis_tk8.3/generic \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \

include $(ADMMAKE)/targets.mak
