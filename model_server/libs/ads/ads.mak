include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/ads.lib

OBJS= \
	$(INTDIR)/argParam.obj \
	$(INTDIR)/argPrmAssign.obj \
	$(INTDIR)/cstring.obj \
	$(INTDIR)/dataSort.obj \
	$(INTDIR)/heapSort.obj \
	$(INTDIR)/indAccess.obj \
	$(INTDIR)/indFSet.obj \
	$(INTDIR)/intrCont.obj \
	$(INTDIR)/listitem.obj \
	$(INTDIR)/parray.obj \
	$(INTDIR)/pstack.obj \
	$(INTDIR)/pqueue.obj

VPATH=../../../gala/extern/ads/src;\
      $(PATH2BS)/gala/extern/ads/src

CPP_INCLUDES=\
/I ../../../gala/extern/ads/include \
/I ../../../gala/extern/osport/include \
/I ../../../machine/include \
/I $(PATH2BS)/gala/extern/ads/include \
/I $(PATH2BS)/gala/extern/osport/include \
/I $(PATH2BS)/machine/include \

include $(ADMMAKE)/targets.mak
