# Microsoft Developer Studio Generated NMAKE File, Based on pathcvt.dsp
ifeq "$(CFG)" ""
CFG=pathcvt - Win32 Debug
$(warning No configuration specified. Defaulting to pathcvt - Win32 Debug.)
endif 

ifneq "$(CFG)" "pathcvt - Win32 Release" 
ifneq "$(CFG)" "pathcvt - Win32 Debug"
$(warning Invalid configuration "$(CFG)" specified.)
$(warning You can specify a configuration when running NMAKE)
$(warning by defining the macro CFG on the command line. For example:)
$(warning )
$(warning NMAKE /f "pathcvt.mak" CFG="pathcvt - Win32 Debug")
$(warning )
$(warning Possible choices for configuration are:)
$(warning )
$(warning "pathcvt - Win32 Release" (based on "Win32 (x86) Console Application"))
$(warning "pathcvt - Win32 Debug" (based on "Win32 (x86) Console Application"))
$(warning )
$(error An invalid configuration is specified.)
endif 
endif 

CPP=cl.exe
LINK32=link.exe
VPATH=./src

ifeq  "$(CFG)" "pathcvt - Win32 Release"

OUTDIR=Release
INTDIR=Release

CPP_PROJ=/nologo /ML /W3 /GX /O2 /I include /D "WIN32" /D "NDEBUG" \
	/D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)/pathcvt.pch" \
	/Fo"$(INTDIR)/" /c 

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib \
	advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib \
	odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib \
	advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib \
	/nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)/pathcvt.pdb" \
	/machine:I386 /out:"$(OUTDIR)/pathcvt.exe" 

else #  "$(CFG)" == "pathcvt - Win32 Debug"

OUTDIR=Debug
INTDIR=Debug

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /I include /D "WIN32" /D "_DEBUG" \
	/D "_CONSOLE" /D "_MBCS" /Fp$(INTDIR)/pathcvt.pch /Fo"$(INTDIR)/" \
	/GZ /c 

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib \
	shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib \
	user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib \
	uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes \
	/pdb:"$(OUTDIR)/pathcvt.pdb" /debug /machine:I386 /out:"$(OUTDIR)/pathcvt.exe" /pdbtype:sept 

endif 

ALL : $(OUTDIR)/pathcvt.exe

LINK32_OBJS= \
	$(INTDIR)/pathcvt.obj \
	$(INTDIR)/StdAfx.obj

$(OUTDIR)/pathcvt.exe : $(OUTDIR) $(LINK32_OBJS)
	$(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)

PATHTOROOT=..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/targets.mak

