include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/dd.lib

OBJS= \
	$(INTDIR)/copy_member.obj \
	$(INTDIR)/ddHeader.h.obj \
	$(INTDIR)/ddIfl.h.obj \
	$(INTDIR)/dd_check_calls.obj \
	$(INTDIR)/dd_check_macros.obj \
	$(INTDIR)/dd_db.obj \
	$(INTDIR)/dd_get_func_called.obj \
	$(INTDIR)/dd_get_struct_rel.obj \
	$(INTDIR)/dd_or_xref_node.h.obj \
	$(INTDIR)/dd_set_baseclass.obj \
	$(INTDIR)/dd_smt_util.obj \
	$(INTDIR)/dd_style.obj \
	$(INTDIR)/dd_type_string.obj \
	$(INTDIR)/dd_utils.obj \
	$(INTDIR)/ddbuild.obj \
	$(INTDIR)/ddfunc.obj \
	$(INTDIR)/ddict.h.obj \
	$(INTDIR)/ioErrorCheck.h.obj \
	$(INTDIR)/moved_from_xref.obj \
	$(INTDIR)/parse_cplusplus_typename.obj \
	$(INTDIR)/xref.h.obj \
	$(INTDIR)/xrefMapper.h.obj \
	$(INTDIR)/xref_dd.obj \
	$(INTDIR)/xref_util.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../api/include \
/I ../../ast/include \
/I ../../dfa/include \
/I ../../driver_aset/include \
/I ../../driver_mb/include \
/I ../../interface/include \
/I ../../ldr/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../process/include \
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
/I ../../util/include \
/I ../../xref/include \
/I ../../../dis_tcl8.3/generic \
/I ../../../dis_tk8.3/generic \
/I ../../../gen/include \
/I ../../../gcc/src \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../stream_message/include \
/I ../../../xxinterface/include \
/I $(PATH2BS)/dis_tcl8.3/generic \
/I $(PATH2BS)/dis_tk8.3/generic \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/gcc/src \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/stream_message/include \
/I $(PATH2BS)/xxinterface/include \
 

include $(ADMMAKE)/targets.mak
