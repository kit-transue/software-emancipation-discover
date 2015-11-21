# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

ifeq "$(CFG)" ""
CFG=dismb_-_Win32_Debug
$(warning No configuration specified.  Defaulting to dismb_-_Win32_Debug.)
endif 

ifneq "$(CFG)" "dismb_-_Win32_Release" 
ifneq "$(CFG)" "dismb_-_Win32_Debug"
$(warning Invalid configuration "$(CFG)" specified.)
$(warning You can specify a configuration when running NMAKE on this makefile)
$(warning by defining the macro CFG on the command line.  For example:)
$(warning )
$(warning NMAKE /f "dismb.mak" CFG="dismb_-_Win32_Debug")
$(warning )
$(warning Possible choices for configuration are:)
$(warning )
$(warning "dismb_-_Win32_Release" (based on "Win32 (x86) Console Application"))
$(warning "dismb_-_Win32_Debug" (based on "Win32 (x86) Console Application"))
$(warning )
$(error An invalid configuration is specified.)
endif 
endif 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "dismb_-_Win32_Debug"
CPP=cl.exe
RSC=rc.exe
LINK32=link.exe

ifeq  "$(CFG)" "dismb_-_Win32_Release"

OUTDIR=Release
INTDIR=Release

LINK32_FLAGS=libcmt.lib kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:$(OUTDIR)/dismb.pdb /machine:I386 /out:$(OUTDIR)/dismb.exe 

else #  "$(CFG)" == "dismb_-_Win32_Debug"

OUTDIR=Debug
INTDIR=Debug

LINK32_FLAGS=libcmtd.lib kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:$(OUTDIR)/dismb.pdb /debug /machine:I386 /out:$(OUTDIR)/dismb.exe

endif 

ALL : $(OUTDIR)/dismb.exe

CLEAN : 
	-@rm $(OUTDIR)/dismb.exe
	-@rm $(OUTDIR)/dismb.ilk
	-@rm $(OUTDIR)/dismb.pdb

$(OUTDIR) :
	$(if $(shell test -e $(OUTDIR)),,-@echo "Creating $(OUTDIR) directory...";mkdir $(OUTDIR))
 
LINK32_OBJS= \
	./libs/source/$(INTDIR)/source.lib\
	./libs/ads/$(INTDIR)/ads.lib\
	./libs/osport/$(INTDIR)/osport.lib

$(OUTDIR)/dismb.exe : $(OUTDIR) $(LINK32_OBJS)
	$(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)

# End Project
################################################################################
