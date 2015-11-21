# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

ifeq "$(CFG)" ""
CFG=simplify_-_Win32_Debug
$(warning No configuration specified.  Defaulting to simplify_-_Win32_Debug.)
endif 

ifneq "$(CFG)" "simplify_-_Win32_Release" 
ifneq "$(CFG)" "simplify_-_Win32_Debug"
$(warning Invalid configuration "$(CFG)" specified.)
$(warning You can specify a configuration when running NMAKE on this makefile)
$(warning by defining the macro CFG on the command line.  For example:)
$(warning )
$(warning NMAKE /f "simplify.mak" CFG="simplify_-_Win32_Debug")
$(warning )
$(warning Possible choices for configuration are:)
$(warning )
$(warning "simplify_-_Win32_Release" (based on "Win32 (x86) Console Application"))
$(warning "simplify_-_Win32_Debug" (based on "Win32 (x86) Console Application"))
$(warning )
$(error An invalid configuration is specified.)
endif 
endif 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "simplify_-_Win32_Debug"
CPP=cl.exe
RSC=rc.exe
LINK32=link.exe

ifeq  "$(CFG)" "simplify_-_Win32_Release"

OUTDIR=Release
INTDIR=Release

else #  "$(CFG)" == "simplify_-_Win32_Debug"

OUTDIR=Debug
INTDIR=Debug

endif 

LINK32_FL=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib msvcrt.lib \
 odbccp32.lib ws2_32.lib Mpr.lib /nologo /subsystem:console /incremental:yes /nodefaultlib:libcmt \
 /pdb:$(OUTDIR)/simplify.pdb /machine:IX86 \
 /out:$(OUTDIR)/simplify.exe 

ifeq "$(CFG)" "simplify_-_Win32_Debug"
	LINK32_FLAGS=$(LINK32_FL) /debug
else
	LINK32_FLAGS=$(LINK32_FL)
endif


ALL : $(OUTDIR)/simplify.exe

CLEAN : 
	-@rm $(OUTDIR)/simplify.exe
	-@rm $(OUTDIR)/simplify.ilk
	-@rm $(OUTDIR)/simplify.pdb

$(OUTDIR) :
	$(if $(shell test -e $(OUTDIR)),,-@echo "Creating $(OUTDIR) directory...";mkdir $(OUTDIR))

LINK32_OBJS= \
	./libs/source/$(INTDIR)/source.lib \
	./libs/machine/$(INTDIR)/machine.lib \
	./libs/osport/$(INTDIR)/osport.lib \
	./libs/if_parser/$(INTDIR)/if_parser.lib \
	./libs/ads/$(INTDIR)/ads.lib

$(OUTDIR)/simplify.exe : $(OUTDIR) $(DEF_FILE) $(LINK32_OBJS)
	$(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)

