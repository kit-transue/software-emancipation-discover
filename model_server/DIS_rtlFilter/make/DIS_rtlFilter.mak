include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/DIS_rtlFilter.lib

OBJS= \
	$(INTDIR)/rtlFilter.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../obj/include \
/I ../../util/include \
/I ../../../gen/include \
/I ../../../gt/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../xincludes \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/gt/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/xincludes \

include $(ADMMAKE)/targets.mak
