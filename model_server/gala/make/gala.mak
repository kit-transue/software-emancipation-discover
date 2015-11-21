include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/gala.lib

OBJS= \
        $(INTDIR)/apl_menu.obj \
        $(INTDIR)/bar.obj \
        $(INTDIR)/centeredLabel.obj \
        $(INTDIR)/client_cmds.obj \
        $(INTDIR)/commandLine.obj \
        $(INTDIR)/controlObjects.obj \
        $(INTDIR)/dialog.obj \
        $(INTDIR)/dialogLayer.obj \
        $(INTDIR)/dis_browser.obj \
        $(INTDIR)/dis_cmds.obj \
        $(INTDIR)/dis_confirm.obj \
        $(INTDIR)/dis_path.obj \
        $(INTDIR)/DIS_view.obj \
        $(INTDIR)/draw.obj \
        $(INTDIR)/editorLayer.obj \
        $(INTDIR)/facet.obj \
        $(INTDIR)/fileChooser.obj \
        $(INTDIR)/genConfirmDialog.obj \
        $(INTDIR)/genDialog.obj \
        $(INTDIR)/globalFuncs.obj \
        $(INTDIR)/gString.obj \
        $(INTDIR)/interp.obj \
        $(INTDIR)/layer.obj \
        $(INTDIR)/paneObjects.obj \
        $(INTDIR)/path.obj \
        $(INTDIR)/prefs.obj \
        $(INTDIR)/rtlClient.obj \
        $(INTDIR)/sash.obj \
        $(INTDIR)/scanner.obj \
        $(INTDIR)/state.obj \
        $(INTDIR)/subwin.obj \
        $(INTDIR)/syspipe.obj \
        $(INTDIR)/tcpServer.obj \
        $(INTDIR)/TextFile.obj \
        $(INTDIR)/toolbar.obj \
        $(INTDIR)/view.obj \
        $(INTDIR)/viewer.obj \
        $(INTDIR)/viewerMenu.obj \
        $(INTDIR)/viewerMenuBar.obj \
        $(INTDIR)/viewerMenuItem.obj \
        $(INTDIR)/tipwin.obj \
        $(INTDIR)/identify.obj \
        $(INTDIR)/SpreadSheet.obj \
        $(INTDIR)/shelltextitem.obj \
        $(INTDIR)/viewerPopupMenuList.obj \
        $(INTDIR)/AttributeEditor.obj \
        $(INTDIR)/gDrawingArea.obj \

VPATH= ../src;\
       ../dis_gala;\
       ../../gala_communications/src

CPP_INCLUDES=\
/I ../.. \
/I ../include \
/I ../../DIS_create/include \
/I ../../api/include \
/I ../../dd/include \
/I ../../driver_aset/include \
/I ../../galaxy_includes/include \
/I ../../gala_communications/include \
/I ../../gra/include \
/I ../../graGala/include \
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
/I ../../smt_metric/include \
/I ../../ste/include \
/I ../../style/include \
/I ../../subsystem/include \
/I ../../ui/include \
/I ../../ui_OODT/include \
/I ../../view/include \
/I ../../view_rtl/include \
/I ../../xref/include \
/I ../../../dis_tcl8.3/generic \
/I ../../../dis_tk8.3/generic \
/I ../../../gala_clients \
/I ../../../gen/include \
/I ../../../gt/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../stream_message/include \
/I $(PATH2BS)/dis_tcl8.3/generic \
/I $(PATH2BS)/dis_tk8.3/generic \
/I $(PATH2BS)/gala_clients \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/gt/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/stream_message/include \

include $(ADMMAKE)/targets.mak
