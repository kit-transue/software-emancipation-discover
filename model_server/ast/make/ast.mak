include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/ast.lib

OBJS= \
	$(INTDIR)/ast_aset.obj \
	$(INTDIR)/ast_dd.obj \
	$(INTDIR)/ast_shared_master.obj \
	$(INTDIR)/ast_xref.obj \
	$(INTDIR)/astImport.obj \
	$(INTDIR)/astTree.h.obj \
	$(INTDIR)/astTreeh.obj \
	$(INTDIR)/get_id.obj \
	$(INTDIR)/iMaster.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../cmd/include \
/I ../../dd/include \
/I ../../driver_aset/include \
/I ../../ldr/include \
/I ../../obj/include \
/I ../../process/include \
/I ../../rtl/include \
/I ../../smt/include \
/I ../../ste/include \
/I ../../ui_viewer/include \
/I ../../xref/include \
/I ../../../gcc/src \
/I ../../../gen/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../xincludes \
/I $(PATH2BS)/gcc/src \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/xincludes \

include $(ADMMAKE)/targets.mak
