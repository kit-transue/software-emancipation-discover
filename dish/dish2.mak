# Microsoft Developer Studio Generated NMAKE File, Based on dish2.dsp
ifeq "$(CFG)" ""
CFG=dish2_-_Win32_Debug
$(warning No configuration specified. Defaulting to dish2_-_Win32_Debug.)
endif 

ifneq "$(CFG)" "dish2_-_Win32_Release" 
ifneq "$(CFG)" "dish2_-_Win32_Debug"
$(warning Invalid configuration "$(CFG)" specified.)
$(warning You can specify a configuration when running NMAKE)
$(warning by defining the macro CFG on the command line. For example:)
$(warning )
$(warning NMAKE /f "dish2.mak" CFG="dish2_-_Win32_Debug")
$(warning )
$(warning Possible choices for configuration are:)
$(warning )
$(warning "dish2_-_Win32_Release" (based on "Win32 (x86) Console Application"))
$(warning "dish2_-_Win32_Debug" (based on "Win32 (x86) Console Application"))
$(warning )
$(error An invalid configuration is specified.)
endif
endif 

CPP=cl.exe
RSC=rc.exe
LINK32=link.exe

ifeq  "$(CFG)" "dish2_-_Win32_Release"

OUTDIR=Release
INTDIR=Release

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib comctl32.lib netapi32.lib wsock32.lib libcmt.lib /nologo /subsystem:console\
 /incremental:no /pdb:$(OUTDIR)\dish2.pdb /machine:I386\
 /out:$(OUTDIR)\dish2.exe

else # debug

OUTDIR=Debug
INTDIR=Debug

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib comctl32.lib netapi32.lib wsock32.lib libcmtd.lib /nologo\
 /subsystem:console /incremental:no /pdb:$(OUTDIR)\dish2.pdb /debug\
 /machine:I386 /out:$(OUTDIR)\dish2.exe\
 /pdbtype:sept 
endif 

LINK32_OBJS= \
	./libs/nameserver/$(INTDIR)/nameserver.lib \
	./libs/src/$(INTDIR)/src.lib \
	./libs/xxinterface/$(INTDIR)/xxinterface.lib

ALL : $(OUTDIR)\dish2.exe

CLEAN :
	-@rm "$(OUTDIR)\dish2.exe"
	-@rm "$(OUTDIR)\dish2.pdb"

$(OUTDIR) :
	$(if $(shell test -e $(OUTDIR)),,-@echo "Creating $(OUTDIR) directory...";mkdir $(OUTDIR))

$(OUTDIR)\dish2.exe : $(OUTDIR) $(LINK32_OBJS)
	$(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)

