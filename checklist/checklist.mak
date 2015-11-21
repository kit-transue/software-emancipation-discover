# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

ifeq "$(CFG)" ""
CFG=checklist_-_Win32_Debug
$(warning No configuration specified.  Defaulting to checklist_-_Win32_Debug.)
endif 

ifneq "$(CFG)" "checklist_-_Win32_Release" 
ifneq "$(CFG)" "checklist_-_Win32_Debug"
$(warning Invalid configuration "$(CFG)" specified.)
$(warning You can specify a configuration when running NMAKE on this makefile)
$(warning by defining the macro CFG on the command line.  For example:)
$(warning )
$(warning NMAKE /f "checklist.mak" CFG="checklist_-_Win32_Debug")
$(warning )
$(warning Possible choices for configuration are:)
$(warning )
$(warning "checklist_-_Win32_Release" (based on "Win32 (x86) Console Application"))
$(warning "checklist_-_Win32_Debug" (based on "Win32 (x86) Console Application"))
$(warning )
$(error An invalid configuration is specified.)
endif 
endif 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "checklist_-_Win32_Debug"
CPP=cl.exe
RSC=rc.exe
LINK32=link.exe

ifeq  "$(CFG)" "checklist_-_Win32_Release"

OUTDIR=Release
INTDIR=Release

CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE"\
 /Fo"$(INTDIR)/" /c 
	
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib libcmt.lib libcimt.lib wsock32.lib mpr.lib /nologo /subsystem:console /incremental:no\
 /pdb:$(OUTDIR)/checklist.pdb /machine:I386 /out:$(OUTDIR)/checklist.exe 

else #  "$(CFG)" == "checklist_-_Win32_Debug"

OUTDIR=Debug
INTDIR=Debug

CPP_PROJ=/nologo /MT /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /Fo"$(INTDIR)/" /c 
	
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib libcmt.lib libcimt.lib wsock32.lib mpr.lib /nodefaultlib:libcimtd.lib /nologo /subsystem:console /incremental:yes\
 /pdb:$(OUTDIR)/checklist.pdb /debug /machine:I386\
 /out:$(OUTDIR)/checklist.exe 

endif 

ALL : $(OUTDIR)/checklist.exe

CLEAN : 
	-@rm $(OUTDIR)/checklist.exe
	-@rm $(OUTDIR)/checklist.ilk
	-@rm $(OUTDIR)/checklist.pdb
                                    
$(OUTDIR) :
	$(if $(shell test -e $(OUTDIR)),,-@echo "Creating $(OUTDIR) directory...";mkdir $(OUTDIR))

LINK32_OBJS= \
	./libs/src/$(OUTDIR)/src.lib \
	./libs/nihcl/$(OUTDIR)/nihcl.lib

$(OUTDIR)/checklist.exe : $(OUTDIR) $(LINK32_OBJS)
	$(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)

# End Project
################################################################################
