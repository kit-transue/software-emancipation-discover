include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/subsystem.lib

OBJS= \
	$(INTDIR)/autosubsys.obj \
	$(INTDIR)/bit_array.obj \
	$(INTDIR)/Entity.obj \
	$(INTDIR)/groupHdr.obj \
	$(INTDIR)/groupTree.obj \
	$(INTDIR)/refs_and_weights.obj \
	$(INTDIR)/subsys.obj \
	$(INTDIR)/subsys_decomposition.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../api/include \
/I ../../cmd/include \
/I ../../dd/include \
/I ../../driver_aset/include \
/I ../../gra/include \
/I ../../interface/include \
/I ../../ldr/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../project/include \
/I ../../project_pdf/include \
/I ../../rtl/include \
/I ../../save/include \
/I ../../search/include \
/I ../../smt/include \
/I ../../ste/include \
/I ../../style/include \
/I ../../ui/include \
/I ../../ui_OODT/include \
/I ../../ui_browser/include \
/I ../../view/include \
/I ../../view_rtl/include \
/I ../../xref/include \
/I ../../../gen/include \
/I ../../../gt/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../paracancel/include \
/I ../../../stream_message/include \
/I ../../../xxinterface/include \
/I ../../../xincludes \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/gt/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/paracancel/include \
/I $(PATH2BS)/stream_message/include \
/I $(PATH2BS)/xxinterface/include \
/I $(PATH2BS)/xincludes \

include $(ADMMAKE)/targets.mak
