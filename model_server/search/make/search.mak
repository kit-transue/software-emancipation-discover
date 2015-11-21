include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/search.lib

OBJS= \
	$(INTDIR)/avl_tree.h.obj \
	$(INTDIR)/binsearch.h.obj \
	$(INTDIR)/hashtable.h.obj \
	$(INTDIR)/linked_list.h.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../../gen/include \
/I $(PATH2BS)/gen/include \

include $(ADMMAKE)/targets.mak
