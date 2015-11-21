include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/style.lib

OBJS= \
	$(INTDIR)/sty_category_handler.obj \
	$(INTDIR)/sty_category_interface.obj \
	$(INTDIR)/sty_default_tables.obj \
	$(INTDIR)/sty_style_handler.obj \
	$(INTDIR)/styAppCategory.obj \
	$(INTDIR)/styAppStyle.obj \
	$(INTDIR)/styCategory.obj \
	$(INTDIR)/style_copy_member.obj \
	$(INTDIR)/style_hooks.obj \
	$(INTDIR)/style_restore.obj \
	$(INTDIR)/style_test.obj \
	$(INTDIR)/stySlot.obj \
	$(INTDIR)/stySlotTable.obj \
	$(INTDIR)/styStyle.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../dd/include \
/I ../../driver_aset/include \
/I ../../gra/include \
/I ../../ldr/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../report/include \
/I ../../rtl/include \
/I ../../search/include \
/I ../../smt/include \
/I ../../ste/include \
/I ../../ste_disstub/include \
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
