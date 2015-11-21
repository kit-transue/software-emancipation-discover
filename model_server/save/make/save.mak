include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/save.lib

OBJS= \
	$(INTDIR)/db_buffer.h.obj \
	$(INTDIR)/db_def.obj \
	$(INTDIR)/db_intern.h.obj \
	$(INTDIR)/db_io.obj \
	$(INTDIR)/db_read_src_from_pset.obj \
	$(INTDIR)/db_restore.obj \
	$(INTDIR)/db_save.obj \
	$(INTDIR)/db_util.obj \
	$(INTDIR)/save.h.obj \
	$(INTDIR)/save1.obj \
	$(INTDIR)/saveXref.obj \
	$(INTDIR)/save_close.obj \
	$(INTDIR)/save_def.obj \
	$(INTDIR)/save_io.obj \
	$(INTDIR)/save_main.obj \
	$(INTDIR)/save_open.obj \
	$(INTDIR)/save_save.obj \
	$(INTDIR)/save_text.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../api/include \
/I ../../assoc/include \
/I ../../dd/include \
/I ../../dfa/include \
/I ../../driver_aset/include \
/I ../../gedge/include \
/I ../../interface/include \
/I ../../ldr/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../project/include \
/I ../../project_pdf/include \
/I ../../rtl/include \
/I ../../smt/include \
/I ../../ste/include \
/I ../../style/include \
/I ../../subsystem/include \
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
