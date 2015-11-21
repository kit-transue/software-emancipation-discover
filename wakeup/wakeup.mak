# Microsoft Developer Studio Generated NMAKE File, Based on wakeup.dsp
ifeq "$(CFG)" ""
CFG=wakeup_-_Win32_Debug
$(warning No configuration specified. Defaulting to wakeup_-_Win32_Debug.)
endif 

ifneq "$(CFG)" "wakeup_-_Win32_Release" 
ifneq "$(CFG)" "wakeup_-_Win32_Debug"
$(warning Invalid configuration "$(CFG)" specified.)
$(warning You can specify a configuration when running NMAKE)
$(warning by defining the macro CFG on the command line. For example:)
$(warning )
$(warning NMAKE /f "wakeup.mak" CFG="wakeup_-_Win32_Debug")
$(warning )
$(warning Possible choices for configuration are:)
$(warning )
$(warning "wakeup_-_Win32_Release" (based on "Win32 (x86) Console Application"))
$(warning "wakeup_-_Win32_Debug" (based on "Win32 (x86) Console Application"))
$(warning )
$(error An invalid configuration is specified.)
endif 
endif 

CPP=cl.exe
RSC=rc.exe
LINK32=link.exe

ifeq  "$(CFG)" "wakeup_-_Win32_Release"

OUTDIR=Release
INTDIR=Release

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D\
 "_MBCS" /Fp$(INTDIR)/wakeup.pch /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /FD /c 
	
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:$(OUTDIR)/wakeup.pdb /machine:I386 /out:$(OUTDIR)/wakeup.exe 

else #  "$(CFG)" == "wakeup_-_Win32_Debug"

OUTDIR=Debug
INTDIR=Debug

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /D "_MBCS" /Fp$(INTDIR)/wakeup.pch /YX /Fo"$(INTDIR)/" /c 
	
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:$(OUTDIR)/wakeup.pdb /debug /machine:I386 /out:$(OUTDIR)/wakeup.exe\
 /pdbtype:sept 

endif 

LINK32_OBJS= $(INTDIR)/wakeup.obj

ALL : $(OUTDIR)/wakeup.exe

CLEAN :
	-@rm $(INTDIR)/vc50.pdb
	-@rm $(INTDIR)/wakeup.obj
	-@rm $(OUTDIR)/wakeup.exe
	-@rm $(OUTDIR)/wakeup.ilk
	-@rm $(OUTDIR)/wakeup.pdb

$(INTDIR) :
	$(if $(shell test -e $(INTDIR)),,-@echo "Creating $(INTDIR) directory...";mkdir $(INTDIR))

$(INTDIR)/wakeup.obj : $(INTDIR)
	$(CPP) $(CPP_PROJ) ./src/wakeup.c

$(OUTDIR)/wakeup.exe : $(OUTDIR) $(LINK32_OBJS)
	$(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)

