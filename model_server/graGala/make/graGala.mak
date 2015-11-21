include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/graGala.lib

OBJS= \
	$(INTDIR)/gra_galaInterface.obj \
	$(INTDIR)/gra_galaSymbol.obj \
	$(INTDIR)/gra_galaWorld.obj \
	$(INTDIR)/symParser.yacc.obj \
	$(INTDIR)/symParser.lex.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../galaxy_includes/include \
/I ../../dd/include \
/I ../../dagraph_disgui/include \
/I ../../driver_aset/include \
/I ../../gala/include \
/I ../../gra/include \
/I ../../gra_loop/include \
/I ../../interface/include \
/I ../../ldr/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../project/include \
/I ../../project_pdf/include \
/I ../../rtl/include \
/I ../../smt/include \
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
/I ../../../gen/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I $(PATH2BS)/dis_tcl8.3/generic \
/I $(PATH2BS)/dis_tk8.3/generic \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \

include $(ADMMAKE)/targets.mak

################################################################################

SOURCE=../src/symParser.y

$(INTDIR)/symParser.yacc.obj : $(INTDIR) $(SOURCE)
	$(MAKE) -f $(ADMMAKE)/makefile.lex\
		CPP=$(CPP)\
		VPATH=$(VPATH)\
		INTDIR=$(INTDIR)\
		CPP_INCLUDES="$(CPP_INCLUDES)"\
		WORK_BASE=symParser yacch

################################################################################

SOURCE=../src/symParser.l

$(INTDIR)/symParser.lex.obj : $(INTDIR) $(SOURCE)
	$(MAKE) -f $(ADMMAKE)/makefile.lex\
		CPP=$(CPP)\
		VPATH=$(VPATH)\
		INTDIR=$(INTDIR)\
		CPP_INCLUDES="$(CPP_INCLUDES)"\
		WORK_BASE=symParser lex_targ

################################################################################
