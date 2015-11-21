include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/els.lib

OBJS= \
	$(INTDIR)/BrowserController.obj \
	$(INTDIR)/BrowserData.obj \
	$(INTDIR)/DisplaySymbol.obj \
	$(INTDIR)/LanguageController.obj \
	$(INTDIR)/LanguageUI.obj \
	$(INTDIR)/elsLanguage.obj \
	$(INTDIR)/str_utils.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../api/include \
/I ../../cmd/include \
/I ../../dd/include \
/I ../../interface/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../project/include \
/I ../../project_pdf/include \
/I ../../rtl/include \
/I ../../smt/include \
/I ../../smt_metric/include \
/I ../../ui/include \
/I ../../util/include \
/I ../../xref/include \
/I ../../../dis_tcl8.3/generic \
/I ../../../dis_tk8.3/generic \
/I ../../../gen/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../stream_message/include \
/I $(PATH2BS)/dis_tcl8.3/generic \
/I $(PATH2BS)/dis_tk8.3/generic \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/stream_message/include \

include $(ADMMAKE)/targets.mak
