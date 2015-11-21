include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/DIS_create.lib

OBJS= \
	$(INTDIR)/boilerplate.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../dd/include \
/I ../../galaxy_includes/include \
/I ../../gala_communications/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../project/include \
/I ../../project_pdf/include \
/I ../../rtl/include \
/I ../../ui/include \
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
