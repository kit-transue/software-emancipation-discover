include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/interface.lib

OBJS= \
	$(INTDIR)/New_RTL_apl.obj \
	$(INTDIR)/assocbridge.obj \
	$(INTDIR)/dd_do_refs.obj \
	$(INTDIR)/dis_view_create.obj \
	$(INTDIR)/errorBrowser.obj \
	$(INTDIR)/error_lex.lex.obj \
	$(INTDIR)/extract.obj \
	$(INTDIR)/messages.obj \
	$(INTDIR)/prompt.obj \
	$(INTDIR)/popup_QueryAndFetch.obj \
	$(INTDIR)/question.obj \
	$(INTDIR)/statistics.obj \
	$(INTDIR)/stubs.obj \
	$(INTDIR)/update_selection.obj 

VPATH=../src

CPP_INCLUDES=\
/I ../.. \
/I ../include \
/I ../../IDE_editor/include \
/I ../../api/include \
/I ../../ast/include \
/I ../../cmd/include \
/I ../../dagraph_disgui/include \
/I ../../dd/include \
/I ../../driver_aset/include \
/I ../../gala/include \
/I ../../galaxy_includes/include \
/I ../../gala_communications/include \
/I ../../graGala/include \
/I ../../gra/include \
/I ../../gra_loop/include \
/I ../../ldr/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../process/include \
/I ../../project/include \
/I ../../project_cm/include \
/I ../../project_pdf/include \
/I ../../rtl/include \
/I ../../save/include \
/I ../../smt/include \
/I ../../smt_metric/include \
/I ../../ste/include \
/I ../../style/include \
/I ../../subsystem/include \
/I ../../ui/include \
/I ../../ui_OODT/include \
/I ../../ui_browser/include \
/I ../../ui_misc/include \
/I ../../ui_viewer/include \
/I ../../view/include \
/I ../../view_rtl/include \
/I ../../xref/include \
/I ../../../dis_tcl8.3/generic \
/I ../../../dis_tk8.3/generic \
/I ../../../gcc/src \
/I ../../../gen/include \
/I ../../../gt/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../stream_message/include \
/I ../../../xincludes \
/I $(PATH2BS)/dis_tcl8.3/generic \
/I $(PATH2BS)/dis_tk8.3/generic \
/I $(PATH2BS)/gcc/src \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/gt/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/stream_message/include \
/I $(PATH2BS)/xincludes \

CPP_PROJ:=$(CPP_PROJ)\
	/D __STDC__=0

include $(ADMMAKE)/targets.mak

################################################################################

SOURCE=../src/error_lex.l

$(INTDIR)/error_lex.lex.obj : $(INTDIR) $(SOURCE)
	$(MAKE) -f $(ADMMAKE)/makefile.lex\
		CPP=$(CPP)\
		VPATH=$(VPATH)\
		INTDIR=$(INTDIR)\
		CPP_INCLUDES="$(CPP_INCLUDES)"\
		WORK_BASE=error_lex lex_targ

################################################################################
