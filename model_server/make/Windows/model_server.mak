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
 /map:"$(INTDIR)/model_server.map" /out:"$(INTDIR)/model_server.exe"\
 
else
ifeq "$(CFG)" "Debug"

INTDIR=./Debug

LINK32_FLAGS=libcmtd.lib libcimt.lib kernel32.lib user32.lib gdi32.lib \
 winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib \
 uuid.lib mpr.lib odbc32.lib odbccp32.lib libcmt.lib libcimt.lib wsock32.lib \
 $(PATH2BS)/extlib/NT/vgalaxy_vc6.lib netapi32.lib comctl32.lib /nologo\
 /subsystem:console /incremental:no /pdb:"$(INTDIR)/model_server.pdb" /debug\
 /machine:I386 /nodefaultlib:"libcmtd.lib" /nodefaultlib:"libcimtd.lib"\
 /nodefaultlib:"libc.lib" /nodefaultlib:"msvcrt.lib" /nodefaultlib:"libcd.lib"\
 /out:"$(INTDIR)/model_server.exe" 

endif

endif

ALL : $(INTDIR)/model_server.exe

CLEAN : 
	-@erase "$(INTDIR)\model_server.exe"
	-@erase "$(INTDIR)\model_server.pdb"

$(INTDIR) :
	-@mkdir $(INTDIR)

LINK32=link.exe
LINK32_OBJS= \
	../../OODT/make/$(INTDIR)/OODT.lib \
	../../api/make/$(INTDIR)/api.lib \
	../../api_pset/make/$(INTDIR)/api_pset.lib \
	../../assoc/make/$(INTDIR)/assoc.lib \
	../../ast/make/$(INTDIR)/ast.lib \
	../../dd/make/$(INTDIR)/dd.lib \
	../../debug/make/$(INTDIR)/debug.lib \
	../../dfa/make/$(INTDIR)/dfa.lib \
	../../driver/make/$(INTDIR)/driver.lib \
	../../driver_mb/make/$(INTDIR)/driver_mb.lib \
	../../driver_model_server/make/$(INTDIR)/driver_model_server.lib \
	../../els/make/$(INTDIR)/els.lib \
	../../machine_prefs/make/$(INTDIR)/machine_prefs.lib \
	../../obj/make/$(INTDIR)/obj.lib \
	../../process/make/$(INTDIR)/process.lib \
	../../project/make/$(INTDIR)/project.lib \
	../../project_file/make/$(INTDIR)/project_file.lib \
	../../project_pdf/make/$(INTDIR)/project_pdf.lib \
	../../rtl/make/$(INTDIR)/rtl.lib \
	../../save/make/$(INTDIR)/save.lib \
	../../search/make/$(INTDIR)/search.lib \
	../../smt/make/$(INTDIR)/smt.lib \
	../../smt_metric/make/$(INTDIR)/smt_metric.lib \
	../../to_cpp/make/$(INTDIR)/to_cpp.lib \
	../../util/make/$(INTDIR)/util.lib \
	../../version/make/$(INTDIR)/version.lib \
	../../xref/make/$(INTDIR)/xref.lib \
	../../libs/ads/$(INTDIR)/ads.lib \
	../../libs/gen/$(INTDIR)/gen.lib \
	../../libs/if_parser/$(INTDIR)/if_parser.lib \
	../../libs/machine/$(INTDIR)/machine.lib \
	../../libs/nameserver/$(INTDIR)/nameserver.lib \
	../../libs/nihcl/$(INTDIR)/nihcl.lib \
	../../libs/osport/$(INTDIR)/osport.lib \
	../../libs/stream_message/$(INTDIR)/stream_message.lib \
	../../libs/xxinterface/$(INTDIR)/xxinterface.lib \
	$(PATH2BS)/FLEXlm/flexlm/current/i86_n3/$(LMGRLIB) \
	$(PATH2BS)/FLEXlm/flexlm/current/i86_n3/lm_new.obj \
	$(PATH2BS)/dis_tcl8.3/win/tclset.lib \
	$(PATH2BS)/dis_tk8.3/win/tkset.lib \
	$(PATH2BS)/thirdparty/expat-1.95.8/lib/Release/libexpat.lib

$(INTDIR)/model_server.exe : $(INTDIR)
	$(LINK32) $(LINK32_FLAGS) $(PATH2BS)/dis_tk8.3/win/tk.res $(LINK32_OBJS)

