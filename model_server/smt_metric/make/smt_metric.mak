include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/smt_metric.lib

OBJS= \
	$(INTDIR)/MetricElement.obj \
	$(INTDIR)/MetricSet.h.obj \
	$(INTDIR)/metric.obj \
	$(INTDIR)/pref_license.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../dd/include \
/I ../../driver_aset/include \
/I ../../interface/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../project/include \
/I ../../project_pdf/include \
/I ../../rtl/include \
/I ../../smt/include \
/I ../../ste_disstub/include \
/I ../../ui/include \
/I ../../xref/include \
/I ../../../gen/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../stream_message/include \
/I ../../../xxinterface/include \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/stream_message/include \
/I $(PATH2BS)/xxinterface/include \

include $(ADMMAKE)/targets.mak
