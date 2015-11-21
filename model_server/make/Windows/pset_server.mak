# Microsoft Developer Studio Generated NMAKE File, Format Version 4.10

# TARGTYPE "Win32 (x86) Application" 0x0101

ifeq "$(CFG)" ""
CFG=Debug
$(warning No configuration specified.  Defaulting to Debug.)
endif

# SETECH: for license server:
# sets variable LMGRDLLVER and LMGRLIB
include $(PATH2BS)/FLEXlm/flexlm/current_defs_nt

ifneq "$(CFG)" "Release"
 ifneq "$(CFG)" "Debug"
  $(warning Invalid configuration "$(CFG)" specified.
  $(warning You can specify a configuration when running NMAKE on this makefile)
  $(warning by defining the macro CFG on the command line.  For example:)
  $(warning)
  $(warning NMAKE /f <Makefile> CFG="Debug")
  $(warning)
  $(warning Possible choices for configuration are:)
  $(warning)
  $(warning "Release" \(based on "Win32 \(x86\) Static Library"\) )
  $(warning "Debug" \(based on "Win32 \(x86\) Static Library"\) )
  $(warning)
  $(error An invalid configuration is specified.)
 endif
endif

ifeq "$(OS)" "Windows_NT"
NULL=
else
NULL=nul
endif
################################################################################
# Begin Project
CPP=cl.exe

ifeq "$(CFG)" "Release"

INTDIR=./Release

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib mpr.lib\
 $(PATH2BS)/extlib/NT/vgalaxy_vc6.lib libcmt.lib libcimt.lib wsock32.lib\
 netapi32.lib comctl32.lib /nologo /subsystem:console /incremental:no\
 /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcmtd.lib"\
 /nodefaultlib:"libcimtd.lib" /nodefaultlib:"msvcrt.lib" \
 /map:"$(INTDIR)/pset_server.map" /out:"$(INTDIR)/pset_server.exe"\
 
else
ifeq "$(CFG)" "Debug"

INTDIR=./Debug

LINK32_FLAGS=libcmtd.lib libcimt.lib kernel32.lib user32.lib gdi32.lib \
 winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib \
 uuid.lib mpr.lib odbc32.lib odbccp32.lib libcmt.lib libcimt.lib wsock32.lib \
 $(PATH2BS)/extlib/NT/vgalaxy_vc6.lib netapi32.lib comctl32.lib /nologo\
 /subsystem:console /incremental:no /pdb:"$(INTDIR)/pset_server.pdb" /debug\
 /machine:I386 /nodefaultlib:"libcmtd.lib" /nodefaultlib:"libcimtd.lib"\
 /nodefaultlib:"libc.lib" /nodefaultlib:"msvcrt.lib" /nodefaultlib:"libcd.lib"\
 /out:"$(INTDIR)/pset_server.exe" 

endif
endif

ALL : $(INTDIR)/pset_server.exe

CLEAN : 
	-@erase "$(INTDIR)\pset_server.exe"
	-@erase "$(INTDIR)\pset_server.pdb"
	-@erase "$(INTDIR)\pset_server.ilk"

$(INTDIR) :
	-@mkdir $(INTDIR)

LINK32=link.exe
LINK32_OBJS= \
	../../DIS_cancel/make/$(INTDIR)/DIS_cancel.lib \
	../../DIS_cm/make/$(INTDIR)/DIS_cm.lib \
	../../DIS_create/make/$(INTDIR)/DIS_create.lib \
	../../DIS_dormant/make/$(INTDIR)/DIS_dormant.lib \
	../../DIS_main/make/$(INTDIR)/DIS_main.lib \
	../../DIS_pset/make/$(INTDIR)/DIS_pset.lib \
	../../DIS_rtlFilter/make/$(INTDIR)/DIS_rtlFilter.lib \
	../../DIS_ui/make/$(INTDIR)/DIS_ui.lib \
	../../IDE_editor/make/$(INTDIR)/IDE_editor.lib \
	../../OODT/make/$(INTDIR)/OODT.lib \
	../../api/make/$(INTDIR)/api.lib \
	../../api_newui/make/$(INTDIR)/api_newui.lib \
	../../api_pset/make/$(INTDIR)/api_pset.lib \
	../../assoc/make/$(INTDIR)/assoc.lib \
	../../ast/make/$(INTDIR)/ast.lib \
	../../dagraph/make/$(INTDIR)/dagraph.lib \
	../../dagraph_disgui/make/$(INTDIR)/dagraph_disgui.lib \
	../../dagraph_dispmod/make/$(INTDIR)/dagraph_dispmod.lib \
	../../dd/make/$(INTDIR)/dd.lib \
	../../dfa/make/$(INTDIR)/dfa.lib \
	../../driver/make/$(INTDIR)/driver.lib \
	../../driver_aset/make/$(INTDIR)/driver_aset.lib \
	../../driver_mb/make/$(INTDIR)/driver_mb.lib \
	../../edge/make/$(INTDIR)/edge.lib \
	../../els/make/$(INTDIR)/els.lib \
	../../gala/make/$(INTDIR)/gala.lib \
	../../gala_communications/make/$(INTDIR)/gala_communications.lib \
	../../gala_machine/make/$(INTDIR)/gala_machine.lib \
	../../gedge/make/$(INTDIR)/gedge.lib \
	../../graGala/make/$(INTDIR)/graGala.lib \
	../../interface/make/$(INTDIR)/interface.lib \
	../../lde/make/$(INTDIR)/lde.lib \
	../../ldr/make/$(INTDIR)/ldr.lib \
	../../machine_prefs/make/$(INTDIR)/machine_prefs.lib \
	../../obj/make/$(INTDIR)/obj.lib \
	../../partition/make/$(INTDIR)/partition.lib \
	../../process/make/$(INTDIR)/process.lib \
	../../project/make/$(INTDIR)/project.lib \
	../../project_cm/make/$(INTDIR)/project_cm.lib \
	../../project_file/make/$(INTDIR)/project_file.lib \
	../../project_pdf/make/$(INTDIR)/project_pdf.lib \
	../../project_ui/make/$(INTDIR)/project_ui.lib \
	../../report/make/$(INTDIR)/report.lib \
	../../rtl/make/$(INTDIR)/rtl.lib \
	../../save/make/$(INTDIR)/save.lib \
	../../search/make/$(INTDIR)/search.lib \
	../../smt/make/$(INTDIR)/smt.lib \
	../../smt_metric/make/$(INTDIR)/smt_metric.lib \
	../../ste/make/$(INTDIR)/ste.lib \
	../../ste_disstub/make/$(INTDIR)/ste_disstub.lib \
	../../style/make/$(INTDIR)/style.lib \
	../../subsystem/make/$(INTDIR)/subsystem.lib \
	../../test/make/$(INTDIR)/test.lib \
	../../to_cpp/make/$(INTDIR)/to_cpp.lib \
	../../util/make/$(INTDIR)/util.lib \
	../../view/make/$(INTDIR)/view.lib \
	../../view_disstub/make/$(INTDIR)/view_disstub.lib \
	../../view_rtl/make/$(INTDIR)/view_rtl.lib \
	../../version/make/$(INTDIR)/version.lib \
	../../xref/make/$(INTDIR)/xref.lib \
	../../libs/ads/$(INTDIR)/ads.lib \
	../../libs/gen/$(INTDIR)/gen.lib \
	../../libs/if_parser/$(INTDIR)/if_parser.lib \
        ../../libs/nameserver/$(INTDIR)/nameserver.lib \
	../../libs/nihcl/$(INTDIR)/nihcl.lib \
	../../libs/osport/$(INTDIR)/osport.lib \
	../../libs/stream_message/$(INTDIR)/stream_message.lib \
	../../libs/xxinterface/$(INTDIR)/xxinterface.lib \
	$(PATH2BS)/FLEXlm/flexlm/current/i86_n3/$(LMGRLIB) \
	$(PATH2BS)/FLEXlm/flexlm/current/i86_n3/lm_new.obj \
	$(PATH2BS)/dis_tcl8.3/win/tclset.lib \
	$(PATH2BS)/dis_tk8.3/win/tkset.lib \
	$(PATH2BS)/thirdparty/expat-1.95.8/lib/Release/libexpat.lib \
	$(PATH2BS)/extlib/NT/xerces-c_1.lib 


$(INTDIR)/pset_server.exe : $(INTDIR)
	$(LINK32) $(LINK32_FLAGS) $(PATH2BS)/dis_tk8.3/win/tk.res $(LINK32_OBJS)

