include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/project_file.lib

OBJS= \
	$(INTDIR)/char_based_merge.obj \
	$(INTDIR)/comguids.obj \
	$(INTDIR)/loaded_files.obj \
	$(INTDIR)/projModule.obj \
	$(INTDIR)/proj_cmd.obj \
	$(INTDIR)/proj_module.obj \
	$(INTDIR)/proj_module2.obj \
	$(INTDIR)/proj_path.obj \
	$(INTDIR)/projcm_nt.obj \
	$(INTDIR)/restore.obj

VPATH=  ../src \
	../../IDE/configshell

CPP_INCLUDES=\
/I ../.. \
/I ../include \
/I ../../IDE/configshell \
/I ../../api/include \
/I ../../api_ui/include \
/I ../../ast/include \
/I ../../cmd/include \
/I ../../dd/include \
/I ../../dfa/include \
/I ../../driver_aset/include \
/I ../../driver_mb/include \
/I ../../galaxy_includes/include \
/I ../../gala_communications/include \
/I ../../interface/include \
/I ../../ldr/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../project/include \
/I ../../project_cm/include \
/I ../../project_pdf/include \
/I ../../project_ui/include \
/I ../../rtl/include \
/I ../../save/include \
/I ../../smt/include \
/I ../../smt_metric/include \
/I ../../ste/include \
/I ../../subsystem/include \
/I ../../ui/include \
/I ../../ui_viewer/include \
/I ../../util/include \
/I ../../view/include \
/I ../../xref/include \
/I ../../../dis_tcl8.3/generic \
/I ../../../dis_tk8.3/generic \
/I ../../../gen/include \
/I ../../../gcc/src \
/I ../../../gt/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../stream_message/include \
/I ../../../xincludes \
/I ../../../xxinterface/include \
/I $(PATH2BS)/dis_tcl8.3/generic \
/I $(PATH2BS)/dis_tk8.3/generic \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/gcc/src \
/I $(PATH2BS)/gt/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/stream_message/include \
/I $(PATH2BS)/xincludes \
/I $(PATH2BS)/xxinterface/include \

include $(ADMMAKE)/targets.mak
