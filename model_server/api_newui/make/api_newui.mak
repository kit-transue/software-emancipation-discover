include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/api_newui.lib

OBJS= \
	$(INTDIR)/cliGala.obj \
	$(INTDIR)/cliClient.obj \
	$(INTDIR)/CMIntegratorCtrlr.obj \
	$(INTDIR)/mrg_cmds.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../IDE_editor/include \
/I ../../api/include \
/I ../../cmIntegration/include \
/I ../../dd/include \
/I ../../driver_aset/include \
/I ../../els/include \
/I ../../gala/include \
/I ../../galaxy_includes/include \
/I ../../gala_communications/include \
/I ../../interface/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../ldr/include \
/I ../../project/include \
/I ../../project_pdf/include \
/I ../../rtl/include \
/I ../../save/include \
/I ../../smt/include \
/I ../../smt_metric/include \
/I ../../ste/include \
/I ../../style/include \
/I ../../subsystem/include \
/I ../../ui/include \
/I ../../util/include \
/I ../../view/include \
/I ../../view_rtl/include \
/I ../../xref/include \
/I ../../../dis_tcl8.3/generic \
/I ../../../dis_tk8.3/generic \
/I ../../../gen/include \
/I ../../../gt/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../stream_message/include \
/I ../../../xincludes \
/I $(PATH2BS)/dis_tcl8.3/generic \
/I $(PATH2BS)/dis_tk8.3/generic \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/gt/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/stream_message/include \
/I $(PATH2BS)/xincludes \


include $(ADMMAKE)/targets.mak
