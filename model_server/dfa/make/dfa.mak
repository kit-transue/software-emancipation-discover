include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/dfa.lib

OBJS= \
	$(INTDIR)/dfa.h.obj \
	$(INTDIR)/dfa_build.obj \
	$(INTDIR)/dfa_cli.obj \
	$(INTDIR)/dfa_db.obj \
	$(INTDIR)/dfa_find.obj \
	$(INTDIR)/dfa_flow.obj \
	$(INTDIR)/dfa_graph.obj \
	$(INTDIR)/dfa_instance.obj \
	$(INTDIR)/dfa_smt.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../api/include \
/I ../../ast/include \
/I ../../cmd/include \
/I ../../dd/include \
/I ../../debug/include \
/I ../../driver_aset/include \
/I ../../els/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../project/include \
/I ../../project_pdf/include \
/I ../../rtl/include \
/I ../../save/include \
/I ../../smt/include \
/I ../../smt_metric/include \
/I ../../ui/include \
/I ../../util/include \
/I ../../xref/include \
/I ../../../dis_tcl8.3/generic \
/I ../../../dis_tk8.3/generic \
/I ../../../els/if_parser/include \
/I ../../../gcc/src \
/I ../../../gen/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../stream_message/include \
/I ../../../xxinterface/include \
/I $(PATH2BS)/dis_tcl8.3/generic \
/I $(PATH2BS)/dis_tk8.3/generic \
/I $(PATH2BS)/els/if_parser/include \
/I $(PATH2BS)/gcc/src \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/stream_message/include \
/I $(PATH2BS)/xxinterface/include \


include $(ADMMAKE)/targets.mak
