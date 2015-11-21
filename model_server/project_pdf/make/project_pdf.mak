include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/project_pdf.lib

OBJS= \
	$(INTDIR)/path.h.obj \
	$(INTDIR)/path.yacc.obj \
	$(INTDIR)/path.lex.obj \
	$(INTDIR)/path_simplify.obj \
	$(INTDIR)/path0.h.obj \
	$(INTDIR)/path1.h.obj \
	$(INTDIR)/pathcheck.h.obj \
	$(INTDIR)/patherr.h.obj \
	$(INTDIR)/pathor.h.obj \
	$(INTDIR)/pdf_debug.obj \
	$(INTDIR)/pdf_flat.obj \
	$(INTDIR)/pdf_load.lex.obj \
	$(INTDIR)/pdf_parser.obj \
        $(INTDIR)/pdf_scope.h.obj \
	$(INTDIR)/pdf_selector.h.obj \
	$(INTDIR)/pdf_tree.h.obj \
	$(INTDIR)/proj_hash.h.obj \
	$(INTDIR)/projMap.h.obj \
        $(INTDIR)/scope_cli.obj \
	$(INTDIR)/scopeClone.h.obj \
        $(INTDIR)/scopeLeaf.h.obj \
        $(INTDIR)/scopeMaster.h.obj \
        $(INTDIR)/scopeMgr.h.obj \
        $(INTDIR)/scopeNode.h.obj \
        $(INTDIR)/scopeRoot.h.obj \
        $(INTDIR)/scopeUnit.h.obj \
	$(INTDIR)/util.obj \
	$(INTDIR)/verify_pdf.obj

VPATH=../src 

CPP_INCLUDES=\
/I ../include \
/I ../../api/include \
/I ../../dd/include \
/I ../../driver_aset/include \
/I ../../interface/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../project/include \
/I ../../project_cm/include \
/I ../../rtl/include \
/I ../../smt/include \
/I ../../smt_metric/include \
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
/I $(PATH2BS)/dis_tcl8.3/generic \
/I $(PATH2BS)/dis_tk8.3/generic \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/gt/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/paracancel/include \
/I $(PATH2BS)/stream_message/include \
/I $(PATH2BS)/xincludes \

include $(ADMMAKE)/targets.mak

################################################################################

SOURCE=../src/path.y

$(INTDIR)/path.yacc.obj : $(INTDIR) $(SOURCE)
	$(MAKE) -f $(ADMMAKE)/makefile.lex\
		CPP=$(CPP)\
		VPATH=$(VPATH)\
		INTDIR=$(INTDIR)\
		CPP_INCLUDES="$(CPP_INCLUDES)"\
		WORK_BASE=path yacch

################################################################################

SOURCE=../src/path.l

$(INTDIR)/path.lex.obj : $(INTDIR) $(SOURCE)
	$(MAKE) -f $(ADMMAKE)/makefile.lex\
		CPP=$(CPP)\
		VPATH=$(VPATH)\
		INTDIR=$(INTDIR)\
		CPP_INCLUDES="$(CPP_INCLUDES)"\
		WORK_BASE=path lex_targ

################################################################################

SOURCE=../src/pdf_load.l

$(INTDIR)/pdf_load.lex.obj : $(INTDIR) $(SOURCE)
	$(MAKE) -f $(ADMMAKE)/makefile.lex\
		CPP=$(CPP)\
		VPATH=$(VPATH)\
		INTDIR=$(INTDIR)\
		CPP_INCLUDES="$(CPP_INCLUDES)"\
		WORK_BASE=pdf_load lex_targ

################################################################################
