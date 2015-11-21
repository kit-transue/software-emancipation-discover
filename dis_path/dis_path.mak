# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

ifeq "$(CFG)" ""
CFG=dis_path_-_Win32_Debug
$(warning No configuration specified.  Defaulting to dis_path_-_Win32_Debug.)
endif 

ifneq "$(CFG)" "dis_path_-_Win32_Release" 
ifneq "$(CFG)" "dis_path_-_Win32_Debug"
$(warning Invalid configuration "$(CFG)" specified.)
$(warning You can specify a configuration when running NMAKE on this makefile)
$(warning by defining the macro CFG on the command line.  For example:)
$(warning )
$(warning NMAKE /f "dis_path.mak" CFG="dis_path_-_Win32_Debug")
$(warning )
$(warning Possible choices for configuration are:)
$(warning )
$(warning "dis_path_-_Win32_Release" (based on "Win32 (x86) Console Application"))
$(warning "dis_path_-_Win32_Debug" (based on "Win32 (x86) Console Application"))
$(warning )
$(error An invalid configuration is specified.)
endif
endif 

################################################################################
# Begin Project
CPP=cl.exe
RSC=rc.exe
LINK32=link.exe

ifeq  "$(CFG)" "dis_path_-_Win32_Release"

OUTDIR=Release
INTDIR=Release

CPP_PROJ=/nologo /MD /W3 /GX /Z7 /Ob1 \
 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" \
 /I "$(SYSCC)/include" /Fo"$(INTDIR)/" /c 
	
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:$(OUTDIR)/dis_path.pdb /machine:I386 /out:$(OUTDIR)/dis_path.exe 

else #  "$(CFG)" == "dis_path_-_Win32_Debug"

OUTDIR=Debug
INTDIR=Debug


CPP_PROJ=/nologo /MD /W3 /GX /Z7 /Ob1 \
 /D "_DEBUG" /D "WIN32" /D "_CONSOLE" \
 /I "$(SYSCC)/include" /Fo"$(INTDIR)/" /c 

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:$(OUTDIR)/dis_path.pdb /debug /machine:I386\
 /out:$(OUTDIR)/dis_path.exe 

endif 

LINK32_OBJS= \
	./libs/pdf/$(INTDIR)/pdf.lib \
	$(INTDIR)/dis_path.obj

ALL : $(OUTDIR)/dis_path.exe

$(OUTDIR)/dis_path.exe : $(OUTDIR) $(LINK32_OBJS)
	$(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)

VPATH=src

PATHTOROOT=..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/targets.mak
