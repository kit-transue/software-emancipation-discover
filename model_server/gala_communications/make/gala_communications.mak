include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/gala_communications.lib

OBJS= \
	$(INTDIR)/Application.obj \
	$(INTDIR)/getResource.obj \
	$(INTDIR)/SelectionIterator.obj \
	$(INTDIR)/ResourceNameIterator.obj

CPP_INCLUDES=\
/I ../include \
/I ../../galaxy_includes/include \
/I ../../../dis_tcl8.3/generic \
/I ../../../dis_tk8.3/generic \
/I ../../../gen/include \
/I ../../../machine/include \
/I $(PATH2BS)/dis_tcl8.3/generic \
/I $(PATH2BS)/dis_tk8.3/generic \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/machine/include \


VPATH=../src 

include $(ADMMAKE)/targets.mak
