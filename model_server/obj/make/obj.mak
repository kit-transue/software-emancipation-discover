include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/obj.lib

OBJS= \
	$(INTDIR)/BooleanTree.h.obj \
	$(INTDIR)/NotifySoft.obj \
	$(INTDIR)/Rel_descrip.obj \
	$(INTDIR)/RelSentinel.h.obj \
	$(INTDIR)/SoftAssoc.h.obj \
	$(INTDIR)/SoftId.h.obj \
	$(INTDIR)/appMarker.h.obj \
	$(INTDIR)/app_get_header.obj \
	$(INTDIR)/gettime.obj \
	$(INTDIR)/int_to_string.obj \
	$(INTDIR)/objArr.h.obj \
	$(INTDIR)/objArr_Int.h.obj \
	$(INTDIR)/objCollection_h.obj \
	$(INTDIR)/objCopy.obj \
	$(INTDIR)/objDb.h.obj \
	$(INTDIR)/objDictionary.obj \
	$(INTDIR)/objNameTable.h.obj \
	$(INTDIR)/objOp.obj \
	$(INTDIR)/objOper_aux.obj \
	$(INTDIR)/objOper_h.obj \
	$(INTDIR)/objPropagate.h.obj \
	$(INTDIR)/objRegenerator.h.obj \
	$(INTDIR)/objRelation.h.obj \
	$(INTDIR)/objReport.obj \
	$(INTDIR)/objSet.h.obj \
	$(INTDIR)/objTree.h.obj \
	$(INTDIR)/obj_copy_member.obj \
	$(INTDIR)/obj_mem.obj \
	$(INTDIR)/obj_print.obj \
	$(INTDIR)/rel_walk.obj \
	$(INTDIR)/relArr.obj \
	$(INTDIR)/representation.h.obj \
	$(INTDIR)/string_to_int.obj \
	$(INTDIR)/transaction.obj \
	$(INTDIR)/tree_merge.obj \
	$(INTDIR)/tree_time.obj \
	$(INTDIR)/str_to_int.lex.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../assoc/include \
/I ../../cmd/include \
/I ../../dd/include \
/I ../../driver_aset/include \
/I ../../ldr/include \
/I ../../machine_prefs/include \
/I ../../project/include \
/I ../../project_pdf/include \
/I ../../report/include \
/I ../../rtl/include \
/I ../../smt/include \
/I ../../ste/include \
/I ../../ui/include \
/I ../../view/include \
/I ../../xref/include \
/I ../../../gen/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../stream_message/include \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/stream_message/include

include $(ADMMAKE)/targets.mak

################################################################################

SOURCE=../src/str_to_int.l

$(INTDIR)/str_to_int.lex.obj : $(INTDIR) $(SOURCE)
	$(MAKE) -k -f $(ADMMAKE)/makefile.lex\
		CPP=$(CPP)\
		VPATH=$(VPATH)\
		INTDIR=$(INTDIR)\
                CPP_INCLUDES="$(CPP_INCLUDES)"\
		WORK_BASE=str_to_int lex_targ

################################################################################
