include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/driver_aset.lib

OBJS= \
	$(INTDIR)/autolog.obj \
	$(INTDIR)/c_main.obj \
	$(INTDIR)/cleanup.obj \
	$(INTDIR)/clipboard.obj \
	$(INTDIR)/deepCopier.obj \
	$(INTDIR)/dialog.obj \
	$(INTDIR)/dialog.h.obj \
	$(INTDIR)/dialog_scroll.obj \
	$(INTDIR)/dialog_set_current_window.obj \
        $(INTDIR)/DIS_cm.obj \
        $(INTDIR)/DIS_cockpit.obj \
        $(INTDIR)/DIS_dormant.obj \
        $(INTDIR)/DIS_extract.obj \
        $(INTDIR)/DIS_gpi.obj \
        $(INTDIR)/DIS_main.obj \
        $(INTDIR)/DIS_rtlFilter.obj \
        $(INTDIR)/DIS_ui.obj \
	$(INTDIR)/driver.obj \
	$(INTDIR)/home_proj_selector.obj \
	$(INTDIR)/LayerServer.obj \
	$(INTDIR)/main.obj \
	$(INTDIR)/msgSet.obj \
        $(INTDIR)/regularTimer.obj \
	$(INTDIR)/Notifier.obj \
	$(INTDIR)/objRef.obj \
	$(INTDIR)/OperPoint.obj \
	$(INTDIR)/propSet.obj \
	$(INTDIR)/reference.obj \
	$(INTDIR)/RTListServer.obj \
	$(INTDIR)/scrapbook.obj \
	$(INTDIR)/selList.obj \
	$(INTDIR)/set_homeproj.obj \
	$(INTDIR)/spd_get_selection_array.obj \
	$(INTDIR)/ui.obj \
	$(INTDIR)/view_clean.obj

VPATH=../src

CPP_INCLUDES=\
/I ../.. \
/I ../include \
/I ../../api \
/I ../../api/include \
/I ../../ast/include \
/I ../../cmd/include \
/I ../../dagraph_disgui/include \
/I ../../dd/include \
/I ../../driver/include \
/I ../../driver_mb/include \
/I ../../els/include \
/I ../../gala/include \
/I ../../gala_communications/include \
/I ../../galaxy_includes/include \
/I ../../gedge/include \
/I ../../gra/include \
/I ../../graGala/include \
/I ../../gra_loop/include \
/I ../../interface/include \
/I ../../ldr/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../partition/include \
/I ../../process/include \
/I ../../project/include \
/I ../../project_cm/include \
/I ../../project_pdf/include \
/I ../../report/include \
/I ../../rtl/include \
/I ../../save/include \
/I ../../search/include \
/I ../../smt/include \
/I ../../smt_metric/include \
/I ../../ste/include \
/I ../../ste_disstub/include \
/I ../../style/include \
/I ../../subsystem/include \
/I ../../ui/include \
/I ../../ui_OODT/include \
/I ../../ui_browser/include \
/I ../../ui_viewer/include \
/I ../../util/include \
/I ../../view/include \
/I ../../view_rtl/include \
/I ../../xref/include \
/I ../../../dis_tcl8.3/generic \
/I ../../../dis_tk8.3/generic \
/I ../../../gala_clients \
/I ../../../gcc/src \
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
/I $(PATH2BS)/gala_clients \
/I $(PATH2BS)/gcc/src \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/gt/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/paracancel/include \
/I $(PATH2BS)/stream_message/include \
/I $(PATH2BS)/xincludes \
/I $(PATH2BS)/xxinterface/include \

include $(ADMMAKE)/targets.mak
