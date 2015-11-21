# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

ifeq "$(CFG)" ""
CFG=setfork_-_Win32_Debug
$(warning No configuration specified.  Defaulting to setfork_-_Win32_Debug.)
endif 

ifneq "$(CFG)" "setfork_-_Win32_Release" 
ifneq "$(CFG)" "setfork_-_Win32_Debug"
$(warning Invalid configuration "$(CFG)" specified.)
$(warning You can specify a configuration when running NMAKE on this makefile)
$(warning by defining the macro CFG on the command line.  For example:)
$(warning )
$(warning NMAKE /f "setfork.mak" CFG="setfork_-_Win32_Debug")
$(warning )
$(warning Possible choices for configuration are:)
$(warning )
$(warning "setfork_-_Win32_Release" (based on "Win32 (x86) Console Application"))
$(warning "setfork_-_Win32_Debug" (based on "Win32 (x86) Console Application"))
$(warning )
$(error An invalid configuration is specified.)
endif 
endif 

################################################################################
# Begin Project
CPP=cl.exe
RSC=rc.exe
LINK32=link.exe

ifeq  "$(CFG)" "setfork_-_Win32_Release"

OUTDIR=Release
INTDIR=Release

CPP_PROJ=/nologo /ML /W3 /GX /O2  /D \
 "WIN32" /D "NDEBUG" /D "_CONSOLE" /Fo"$(INTDIR)/" /c 
	
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib msvcrt.lib /nologo /subsystem:console /incremental:no\
 /pdb:$(OUTDIR)/setfork.pdb /machine:I386 /out:$(OUTDIR)/setfork.exe


else #  "$(CFG)" == "setfork_-_Win32_Debug"

OUTDIR=Debug
INTDIR=Debug

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od \
	/D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
	
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib msvcrt.lib /nologo /subsystem:console /incremental:yes\
 /pdb:$(OUTDIR)/setfork.pdb /debug /machine:I386 /out:$(OUTDIR)/setfork.exe

endif 

ALL : $(OUTDIR)/setfork.exe

CLEAN : 
	-@rm $(INTDIR)/vc40.idb
	-@rm $(INTDIR)/vc40.pdb
	-@rm $(OUTDIR)/setfork.exe
	-@rm $(OUTDIR)/setfork.ilk
	-@rm $(OUTDIR)/setfork.pdb

$(OUTDIR) :
	$(if $(shell test -e $(OUTDIR)),,-@echo "Creating $(OUTDIR) directory...";mkdir $(OUTDIR))
 
LINK32_OBJS= \
	./libs/ntfork/$(INTDIR)/ntfork.lib

$(OUTDIR)/setfork.exe : $(OUTDIR) $(LINK32_OBJS)
	$(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)

# End Project
################################################################################
