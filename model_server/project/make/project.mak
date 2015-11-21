include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/project.lib

OBJS= \
	$(INTDIR)/fileCache.h.obj \
	$(INTDIR)/fileEntry.obj \
	$(INTDIR)/fileXref.h.obj \
	$(INTDIR)/glob.obj \
	$(INTDIR)/module_create.obj \
	$(INTDIR)/prj_env_vars.obj \
	$(INTDIR)/proj.h.obj \
	$(INTDIR)/proj_clean.obj \
	$(INTDIR)/proj_restore.obj \
	$(INTDIR)/proj_save.obj \
	$(INTDIR)/projFile.h.obj \
	$(INTDIR)/projHeader.obj \
	$(INTDIR)/projList.obj \
	$(INTDIR)/projectDB.obj \
	$(INTDIR)/projectHeader.h.obj \
	$(INTDIR)/projectModule.h.obj \
	$(INTDIR)/projectNode.h.obj \
	$(INTDIR)/pset_fn.obj \
	$(INTDIR)/report.obj \
	$(INTDIR)/viewable.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../api/include \
/I ../../cmd/include \
/I ../../dd/include \
/I ../../driver_aset/include \
/I ../../driver_mb/include \
/I ../../interface/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../project_pdf/include \
/I ../../project_cm/include \
/I ../../rtl/include \
/I ../../save/include \
/I ../../smt/include \
/I ../../smt_metric/include \
/I ../../ui/include \
/I ../../ui_viewer/include \
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
