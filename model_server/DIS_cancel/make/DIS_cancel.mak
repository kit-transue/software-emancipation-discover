include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/DIS_cancel.lib

OBJS= \
	$(INTDIR)/ServerCancel.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../galaxy_includes/include \
/I ../../gala_communications/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../../gen/include \
/I ../../../machine/include \
/I ../../../paracancel/include \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/paracancel/include \

include $(ADMMAKE)/targets.mak
