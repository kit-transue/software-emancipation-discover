include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/xref.lib

OBJS= \
	$(INTDIR)/Link.h.obj \
	$(INTDIR)/SharedXref.h.obj \
	$(INTDIR)/Xref.obj \
	$(INTDIR)/Xref.h.obj \
	$(INTDIR)/Xref_util.obj \
	$(INTDIR)/XrefQuery.obj \
	$(INTDIR)/XrefTable.h.obj \
	$(INTDIR)/checklinks.obj \
	$(INTDIR)/checkpmod.obj \
	$(INTDIR)/ddKind.h.obj \
	$(INTDIR)/ddKind_readable_names.obj \
	$(INTDIR)/ddSymbol.h.obj \
	$(INTDIR)/link_type_converter.h.obj \
	$(INTDIR)/linkType_selector.obj \
	$(INTDIR)/linkTypes.h.obj \
	$(INTDIR)/print_pmod.obj \
	$(INTDIR)/remove_modules.obj \
	$(INTDIR)/sortlinks.obj \
	$(INTDIR)/symHeaderInfo.h.obj \
	$(INTDIR)/symInd.h.obj \
	$(INTDIR)/symbolArr.h.obj \
	$(INTDIR)/symbolPtr.h.obj \
	$(INTDIR)/symbolScope.h.obj \
	$(INTDIR)/symbolSet.h.obj \
	$(INTDIR)/xrefSymbol.h.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../api/include \
/I ../../ast/include \
/I ../../cmd/include \
/I ../../dd/include \
/I ../../dfa/include \
/I ../../driver_aset/include \
/I ../../driver_mb/include \
/I ../../interface/include \
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
/I ../../subsystem/include \
/I ../../ui/include \
/I ../../ui_browser/include \
/I ../../../gcc/src \
/I ../../../gen/include \
/I ../../../gt/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../paracancel/include \
/I ../../../stream_message/include \
/I ../../../xincludes \
/I ../../../xxinterface/include \
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
