include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/smt.lib

OBJS= \
	$(INTDIR)/get_smt_decl.obj \
	$(INTDIR)/metricSMT.obj \
	$(INTDIR)/smt.obj \
	$(INTDIR)/smt.h.obj \
	$(INTDIR)/smtScd.h.obj \
	$(INTDIR)/smtScdTypes.obj \
	$(INTDIR)/smt_ast.obj \
	$(INTDIR)/smt_ast2.obj \
	$(INTDIR)/smt_categories.obj \
	$(INTDIR)/smt_cppparm.obj \
	$(INTDIR)/smt_ctree.obj \
	$(INTDIR)/smt_cut.obj \
	$(INTDIR)/smt_db.obj \
	$(INTDIR)/smt_get_ast_new.obj \
	$(INTDIR)/smt_get_node_type.obj \
	$(INTDIR)/smt_ifl.obj \
	$(INTDIR)/smt_ifl_CBS.obj \
	$(INTDIR)/smt_interface.obj \
	$(INTDIR)/smt_main.obj \
	$(INTDIR)/smt_map_ast.obj \
	$(INTDIR)/smt_modify.obj \
	$(INTDIR)/smt_replace_text.obj \
	$(INTDIR)/smt_sp.obj \
	$(INTDIR)/smt_spy.yacc.obj \
	$(INTDIR)/smt_spy.lex.obj \
	$(INTDIR)/smt_system.obj \
	$(INTDIR)/smt_tcons.obj \
	$(INTDIR)/smt_translate.obj \
	$(INTDIR)/smt_tree.obj \
	$(INTDIR)/smt_txt.obj \
	$(INTDIR)/smt_view.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../api_ui/include \
/I ../../api/include \
/I ../../ast/include \
/I ../../cmd/include \
/I ../../dd/include \
/I ../../debug/include \
/I ../../dfa/include \
/I ../../driver_aset/include \
/I ../../interface/include \
/I ../../ldr/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../process/include \
/I ../../project/include \
/I ../../project_pdf/include \
/I ../../rtl/include \
/I ../../save/include \
/I ../../smt_metric/include \
/I ../../ste/include \
/I ../../ste_disstub/include \
/I ../../style/include \
/I ../../to_cpp/include \
/I ../../ui/include \
/I ../../util/include \
/I ../../view/include \
/I ../../view_rtl/include \
/I ../../xref/include \
/I ../../../dis_tcl8.3/generic \
/I ../../../dis_tk8.3/generic \
/I ../../../els/if_parser/include \
/I ../../../gcc/src \
/I ../../../gen/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../stream_message/include \
/I ../../../xincludes \
/I $(PATH2BS)/dis_tcl8.3/generic \
/I $(PATH2BS)/dis_tk8.3/generic \
/I $(PATH2BS)/els/if_parser/include \
/I $(PATH2BS)/gcc/src \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/stream_message/include \
/I $(PATH2BS)/xincludes \

include $(ADMMAKE)/targets.mak

################################################################################

SOURCE=../src/smt_spy.y

$(INTDIR)/smt_spy.yacc.obj : $(INTDIR) $(SOURCE)
	$(MAKE) -f $(ADMMAKE)/makefile.lex \
		CPP=$(CPP) \
		VPATH=$(VPATH)\
		INTDIR=$(INTDIR) \
		CPP_INCLUDES="$(CPP_INCLUDES)" \
		WORK_BASE=smt_spy yacch

################################################################################

SOURCE=../src/smt_spy.l

$(INTDIR)/smt_spy.lex.obj : $(INTDIR) $(SOURCE)
	$(MAKE) -f $(ADMMAKE)/makefile.lex \
		CPP=$(CPP) \
		VPATH=$(VPATH)\
		INTDIR=$(INTDIR) \
		CPP_INCLUDES="$(CPP_INCLUDES)" \
		WORK_BASE=smt_spy lex_targ

################################################################################
