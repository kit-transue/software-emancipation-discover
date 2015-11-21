include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/driver_mb.lib

OBJS= \
	$(INTDIR)/disbuild.h.obj \
	$(INTDIR)/disbuild_analysis.h.obj \
	$(INTDIR)/disbuild_cache.h.obj \
	$(INTDIR)/disbuild_flags.h.obj \
	$(INTDIR)/disbuild_update.h.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../dd/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../project/include \
/I ../../project_pdf/include \
/I ../../rtl/include \
/I ../../smt/include \
/I ../../ui/include \
/I ../../util/include \
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
