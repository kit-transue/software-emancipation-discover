include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/DIS_ui.lib

OBJS= \
	$(INTDIR)/StatusDialog.obj \
        $(INTDIR)/ModelAliasList.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../cmd/include \
/I ../../dd/include \
/I ../../driver_aset/include \
/I ../../galaxy_includes/include \
/I ../../gala_communications/include \
/I ../../ldr/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../project/include \
/I ../../project_pdf/include \
/I ../../rtl/include \
/I ../../ste/include \
/I ../../style/include \
/I ../../ui/include \
/I ../../view/include \
/I ../../view_rtl/include \
/I ../../xref/include \
/I ../../../gen/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \

include $(ADMMAKE)/targets.mak
