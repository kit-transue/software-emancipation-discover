PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/nihcl.lib

OBJS= \
	$(INTDIR)/ArrayOb.obj \
	$(INTDIR)/Collection.obj \
	$(INTDIR)/Hash.obj \
	$(INTDIR)/Iterator.obj \
	$(INTDIR)/Object.obj \
	$(INTDIR)/OrderedCltn.obj \
	$(INTDIR)/SeqCltn.obj \
	$(INTDIR)/Set.obj \
	$(INTDIR)/pure_stubs.obj

VPATH=$(PATHTOROOT)/nihcl/src;$(PATH2BS)/nihcl/src

CPP_INCLUDES=\
	/I $(PATHTOROOT)/nihcl/include \
	/I $(PATHTOROOT)/gen/include \
	/I $(PATH2BS)/nihcl/include \
	/I $(PATH2BS)/gen/include

include $(MAKESCRIPTSPATH)/targets.mak
