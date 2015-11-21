include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/api.lib

OBJS= \
	$(INTDIR)/AccessCatalog.obj \
	$(INTDIR)/DataExchange.obj \
	$(INTDIR)/GenericTree.obj \
	$(INTDIR)/Interpreter.obj \
	$(INTDIR)/License.obj \
	$(INTDIR)/Message.obj \
	$(INTDIR)/Properties.obj \
	$(INTDIR)/QueryApp.obj \
	$(INTDIR)/QueryObj.obj \
	$(INTDIR)/QueryTree.obj \
	$(INTDIR)/Server.obj \
	$(INTDIR)/Server_comm_exp.obj \
	$(INTDIR)/SocketClient.obj \
	$(INTDIR)/SubChk.obj \
	$(INTDIR)/TclList.obj \
	$(INTDIR)/WfNode.obj \
	$(INTDIR)/WfTask.obj \
	$(INTDIR)/attribute.obj \
	$(INTDIR)/calls.obj \
	$(INTDIR)/classes.obj \
	$(INTDIR)/cliCommands.obj \
	$(INTDIR)/cliSmt.obj \
	$(INTDIR)/cliser.obj \
	$(INTDIR)/extern_attr.obj \
	$(INTDIR)/externApp.obj \
	$(INTDIR)/externGroup.obj \
	$(INTDIR)/filter.obj \
	$(INTDIR)/filter.yacc.obj \
	$(INTDIR)/format.obj \
	$(INTDIR)/get_sym_attr.obj \
	$(INTDIR)/instance.obj \
	$(INTDIR)/range.lex.obj \
	$(INTDIR)/selection.obj \
	$(INTDIR)/smtMarker.h.obj \
	$(INTDIR)/smtRegion.h.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../ast/include \
/I ../../cmd/include \
/I ../../cpp/include \
/I ../../dd/include \
/I ../../debug/include \
/I ../../dfa/include \
/I ../../driver_aset/include \
/I ../../els/include \
/I ../../interface/include \
/I ../../ldr/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../project/include \
/I ../../project_pdf/include \
/I ../../process/include \
/I ../../rtl/include \
/I ../../save/include \
/I ../../smt/include \
/I ../../smt_metric/include \
/I ../../ste/include \
/I ../../subsystem/include \
/I ../../ui/include \
/I ../../ui_browser/include \
/I ../../ui_viewer/include \
/I ../../xref/include \
/I ../../../dis_tcl8.3/generic \
/I ../../../dis_tk8.3/generic \
/I ../../../expat-1.0/dist/xmlparse \
/I ../../../gcc/src \
/I ../../../gen/include \
/I ../../../gt/include \
/I ../../../machine/include \
/I ../../../nameserver/include \
/I ../../../nihcl/include \
/I ../../../stream_message/include \
/I ../../../xincludes \
/I ../../../xxinterface/include \
/I $(PATH2BS)/dis_tcl8.3/generic \
/I $(PATH2BS)/dis_tk8.3/generic \
/I $(PATH2BS)/expat-1.0/dist/xmlparse \
/I $(PATH2BS)/gcc/src \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/gt/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nameserver/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/stream_message/include \
/I $(PATH2BS)/xincludes \
/I $(PATH2BS)/xxinterface/include

include $(ADMMAKE)/targets.mak

################################################################################

SOURCE=../src/filter.y

$(INTDIR)/filter.yacc.obj : $(INTDIR) $(SOURCE)
	$(MAKE) -f $(ADMMAKE)/makefile.lex\
		CPP=$(CPP)\
		VPATH=$(VPATH)\
		INTDIR=$(INTDIR)\
		CPP_INCLUDES="$(CPP_INCLUDES)"\
		WORK_BASE=filter yacch

################################################################################

SOURCE=../src/range.l

$(INTDIR)/range.lex.obj : $(INTDIR) $(SOURCE)
	$(MAKE) -f $(ADMMAKE)/makefile.lex\
		CPP=$(CPP)\
		VPATH=$(VPATH)\
		INTDIR=$(INTDIR)\
		CPP_INCLUDES="$(CPP_INCLUDES)"\
		WORK_BASE=range lex_targ

################################################################################
