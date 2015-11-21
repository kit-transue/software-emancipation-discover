include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/util.lib

OBJS= \
	$(INTDIR)/BrowserImport.obj \
	$(INTDIR)/CompilerOptions.obj \
	$(INTDIR)/closure.obj \
	$(INTDIR)/filterButtonTable.obj \
	$(INTDIR)/get_related_objs.obj \
	$(INTDIR)/get_simple_name.obj \
	$(INTDIR)/gettext.obj \
	$(INTDIR)/language.obj \
	$(INTDIR)/oodt_class_member_sorter.obj \
	$(INTDIR)/projModule_of_symbol.obj \
	$(INTDIR)/remove_subsys.obj \
	$(INTDIR)/sym_to_proj.obj \
	$(INTDIR)/timer.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../api/include \
/I ../../api_ui/include \
/I ../../dd/include \
/I ../../driver_aset/include \
/I ../../gra/include \
/I ../../interface/include \
/I ../../ldr/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../project/include \
/I ../../project_pdf/include \
/I ../../rtl/include \
/I ../../smt/include \
/I ../../smt_metric/include \
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
