# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

ifeq "$(CFG)" ""
CFG=sevreporter_-_Win32_Debug
$(warning No configuration specified.  Defaulting to sevreporter_-_Win32_Debug.)
endif 

ifneq "$(CFG)" "sevreporter_-_Win32_Release"
ifneq "$(CFG)" "sevreporter_-_Win32_Debug"
$(warning Invalid configuration "$(CFG)" specified.)
$(warning You can specify a configuration when running NMAKE on this makefile)
$(warning by defining the macro CFG on the command line.  For example:)
$(warning )
$(warning NMAKE /f "sevreporter.mak" CFG="sevreporter_-_Win32_Debug")
$(warning )
$(warning Possible choices for configuration are:)
$(warning )
$(warning "sevreporter_-_Win32_Release" (based on "Win32 (x86) Console Application"))
$(warning "sevreporter_-_Win32_Debug" (based on "Win32 (x86) Console Application"))
$(warning )
$(error An invalid configuration is specified.)
endif
endif 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "sevreporter_-_Win32_Debug"
CPP=cl.exe
RSC=rc.exe
LINK32=link.exe

ifeq  "$(CFG)" "sevreporter_-_Win32_Release"

OUTDIR=Release
INTDIR=Release

CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE"\
 /Fo"$(INTDIR)/" /c 
	
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib libcmt.lib /nologo /subsystem:console /incremental:no\
 /pdb:$(OUTDIR)/sevreporter.pdb /machine:I386 /nodefaultlib:libcmtd.lib\
 /out:$(OUTDIR)/sevreporter.exe 

else #  "$(CFG)" == "sevreporter_-_Win32_Debug"

OUTDIR=Debug
INTDIR=Debug

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib libcmtd.lib /nologo /subsystem:console /incremental:yes\
 /pdb:$(OUTDIR)/sevreporter.pdb /debug /machine:I386\
 /nodefaultlib:libcmt.lib /out:$(OUTDIR)/sevreporter.exe 

endif 

LINK32_OBJS= \
	./libs/ads/$(OUTDIR)/ads.lib \
	./libs/osport/$(OUTDIR)/osport.lib \
	./libs/source/$(OUTDIR)/source.lib \
	./libs/gd/$(OUTDIR)/gd.lib \
	./libs/gdchart/$(OUTDIR)/gdchart.lib

ALL : $(OUTDIR)/sevreporter.exe

CLEAN : 
	-@rm $(OUTDIR)/sevreporter.exe
	-@rm $(OUTDIR)/sevreporter.ilk
	-@rm $(OUTDIR)/sevreporter.pdb

$(OUTDIR) :
	$(if $(shell test -e $(OUTDIR)),,-@echo "Creating $(OUTDIR) directory...";mkdir $(OUTDIR))

$(OUTDIR)/sevreporter.exe : $(OUTDIR) $(DEF_FILE) $(LINK32_OBJS)
	$(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)

