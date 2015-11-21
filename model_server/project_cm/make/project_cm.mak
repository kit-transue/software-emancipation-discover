include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/project_cm.lib

OBJS= \
	$(INTDIR)/feedback.obj \
	$(INTDIR)/mrg_group.obj \
	$(INTDIR)/projModule.obj \
	$(INTDIR)/sdo.obj \
	$(INTDIR)/tpopen.obj 

VPATH=../src 

CPP_INCLUDES=\
/I ../include \
/I ../../DIS_cm/include \
/I ../../api/include \
/I ../../api_ui/include \
/I ../../cmd/include \
/I ../../cmIntegration/include \
/I ../../dd/include \
/I ../../driver_aset/include \
/I ../../driver_mb/include \
/I ../../interface/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../project/include \
/I ../../project_pdf/include \
/I ../../rtl/include \
/I ../../save/include \
/I ../../smt/include \
/I ../../smt_metric/include \
/I ../../ui/include \
/I ../../ui_viewer/include \
/I ../../util/include \
/I ../../view/include \
/I ../../xref/include \
/I ../../../gen/include \
/I ../../../gt/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../stream_message/include \
/I ../../../xincludes \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/gt/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/stream_message/include \
/I $(PATH2BS)/xincludes \

include $(ADMMAKE)/targets.mak
