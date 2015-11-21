include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/report.lib

OBJS= \
	$(INTDIR)/ast_report.obj \
	$(INTDIR)/scrapbook-report.obj \
	$(INTDIR)/soft-report.obj \
	$(INTDIR)/ste-report.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../ast/include \
/I ../../dd/include \
/I ../../driver_aset/include \
/I ../../ldr/include \
/I ../../obj/include \
/I ../../rtl/include \
/I ../../smt/include \
/I ../../ste/include \
/I ../../style/include \
/I ../../ui/include \
/I ../../xref/include \
/I ../../../gcc/src \
/I ../../../gen/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../stream_message/include \
/I $(PATH2BS)/gcc/src \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/stream_message/include \

include $(ADMMAKE)/targets.mak
