include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/test.lib

OBJS= \
	$(INTDIR)/cmd_gen.obj \
	$(INTDIR)/cmd_validate.obj

VPATH=../src

CPP_INCLUDES=\
/I ../../cmd/include \
/I ../../dd/include \
/I ../../driver_aset/include \
/I ../../gra/include \
/I ../../graGala/include \
/I ../../gra_loop/include \
/I ../../ldr/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../project/include \
/I ../../project_pdf/include \
/I ../../rtl/include \
/I ../../smt/include \
/I ../../ste/include \
/I ../../style/include \
/I ../../ui/include \
/I ../../view/include \
/I ../../xref/include \
/I ../../../gen/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../stream_message/include \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/stream_message/include \

include $(ADMMAKE)/targets.mak
