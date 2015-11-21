include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/process.lib

OBJS= \
	$(INTDIR)/MemSeg.h.obj \
	$(INTDIR)/SymManager.h.obj \
	$(INTDIR)/finiteStateMachine.obj \
	$(INTDIR)/mManager.h.obj \
	$(INTDIR)/process_stub.obj \
	$(INTDIR)/remangle.obj \
	$(INTDIR)/shared_list.obj \
	$(INTDIR)/shared_malloc.obj \
	$(INTDIR)/signature.obj \
	$(INTDIR)/so_shared.obj \
	$(INTDIR)/tty.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../interface/include \
/I ../../ui/include \
/I ../../../gen/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../stream_message/include \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/stream_message/include \

include $(ADMMAKE)/targets.mak
