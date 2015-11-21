include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/ldr.lib

OBJS= \
	$(INTDIR)/delete_node_views.obj \
	$(INTDIR)/inheritanceSymTreeHdr.obj \
	$(INTDIR)/insert_ldr_headers.obj \
	$(INTDIR)/ldr.obj \
	$(INTDIR)/ldrBrowserHierarchy.obj \
	$(INTDIR)/ldrClipboard.obj \
	$(INTDIR)/ldrDCHierarchy.obj \
	$(INTDIR)/ldrERDHierarchy.obj \
	$(INTDIR)/ldrFuncCallNode.obj \
	$(INTDIR)/ldrHeader.obj \
	$(INTDIR)/ldrHierarchy.obj \
	$(INTDIR)/ldrList.obj \
	$(INTDIR)/ldrNode.obj \
	$(INTDIR)/ldrOODT.obj \
	$(INTDIR)/ldrRTLHierarchy.obj \
	$(INTDIR)/ldrRTLNode.obj \
	$(INTDIR)/ldrScrapbook.obj \
	$(INTDIR)/ldrSelection.obj \
	$(INTDIR)/ldrSmodNode.obj \
	$(INTDIR)/ldrSmodShort.obj \
	$(INTDIR)/ldrSmtFlowchart.obj \
	$(INTDIR)/ldrSmtHierarchy.obj \
	$(INTDIR)/ldrSmtSte_h.obj \
	$(INTDIR)/objRawLdr_h.obj \
	$(INTDIR)/rebuildControl.obj \
	$(INTDIR)/symbolLdrHeader.obj \
	$(INTDIR)/symbolLdrTree.obj

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../api/include \
/I ../../assoc/include \
/I ../../cmd/include \
/I ../../driver_aset/include \
/I ../../dd/include \
/I ../../els/include \
/I ../../gra/include \
/I ../../interface/include \
/I ../../lde/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../project/include \
/I ../../project_pdf/include \
/I ../../rtl/include \
/I ../../save/include \
/I ../../smt/include \
/I ../../smt_metric/include \
/I ../../ste/include \
/I ../../ste_disstub/include \
/I ../../style/include \
/I ../../subsystem/include \
/I ../../ui/include \
/I ../../ui_OODT/include \
/I ../../ui_browser/include \
/I ../../ui_viewer/include \
/I ../../view/include \
/I ../../view_rtl/include \
/I ../../xref/include \
/I ../../../gen/include \
/I ../../../gt/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../paracancel/include \
/I ../../../stream_message/include \
/I ../../../xincludes \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/gt/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/paracancel/include \
/I $(PATH2BS)/stream_message/include \
/I $(PATH2BS)/xincludes \

include $(ADMMAKE)/targets.mak
