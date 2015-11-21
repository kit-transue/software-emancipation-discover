include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/driver_model_server.lib

OBJS= \
	$(INTDIR)/browser_import.obj \
	$(INTDIR)/main.obj \
	$(INTDIR)/stub.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../DIS_create/include \
/I ../../assoc/include \
/I ../../api/include \
/I ../../api_ui/include \
/I ../../cmd/include \
/I ../../dd/include \
/I ../../driver_aset/include \
/I ../../driver_mb/include \
/I ../../els/include \
/I ../../interface/include \
/I ../../ldr/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../process/include \
/I ../../project/include \
/I ../../project_pdf/include \
/I ../../rtl/include \
/I ../../smt/include \
/I ../../smt_metric/include \
/I ../../ste/include \
/I ../../ui/include \
/I ../../xref/include \
/I ../../../dis_tcl8.3/generic \
/I ../../../dis_tk8.3/generic \
/I ../../../gen/include \
/I ../../../gt/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../paracancel/include \
/I ../../../stream_message/include \
/I ../../../xincludes \
/I ../../../xxinterface/include \
/I $(PATH2BS)/dis_tcl8.3/generic \
/I $(PATH2BS)/dis_tk8.3/generic \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/gt/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/paracancel/include \
/I $(PATH2BS)/stream_message/include \
/I $(PATH2BS)/xincludes \
/I $(PATH2BS)/xxinterface/include \

include $(ADMMAKE)/targets.mak
