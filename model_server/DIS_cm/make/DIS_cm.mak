include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/DIS_cm.lib

OBJS= \
	$(INTDIR)/ui_cm_oper.obj \
	$(INTDIR)/ui_cm_get.obj \
	$(INTDIR)/ui_cm_unget.obj \
	$(INTDIR)/ui_cm_put.obj \
	$(INTDIR)/ui_cm_lock.obj \
	$(INTDIR)/ui_cm_unlock.obj \

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../dd/include \
/I ../../gala/include \
/I ../../galaxy_includes/include \
/I ../../gala_communications/include \
/I ../../interface/include \
/I ../../ldr/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../project/include \
/I ../../project_pdf/include \
/I ../../rtl/include \
/I ../../ui/include \
/I ../../util/include \
/I ../../xref/include \
/I ../../../gen/include \
/I ../../../gt/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/gt/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \

include $(ADMMAKE)/targets.mak
