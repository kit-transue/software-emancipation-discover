# Microsoft Developer Studio Generated NMAKE File, Based on dislite.dsp
ifeq "$(CFG)" ""
CFG=Debug
$(warning No configuration specified. Defaulting to dislite_-_Win32_Debug.)
endif 

ifneq "$(CFG)" "Release" 
ifneq "$(CFG)" "Debug"
$(warning Invalid configuration "$(CFG)" specified.)
$(warning You can specify a configuration when running NMAKE)
$(warning by defining the macro CFG on the command line. For example:)
$(warning )
$(warning NMAKE /f "dislite.mak" CFG="Debug")
$(warning )
$(warning Possible choices for configuration are:)
$(warning )
$(warning "Release" (based on "Win32 (x86) Application"))
$(warning "Debug" (based on "Win32 (x86) Application"))
$(warning )
$(error An invalid configuration is specified.)
endif
endif 

CPP=cl.exe
LINK32=link.exe

LINK32_OBJS= \
	$(INTDIR)/dislite.obj \
	$(INTDIR)/dislite_vc.obj \
	./libs/nameserver/$(INTDIR)/nameserver.lib

ifeq  "$(CFG)" "Release"

INTDIR=Release

CPP_PROJ=/nologo /G6 /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" \
 /I "../nameserver/include" /I "$(PATH2BS)/nameserver/include" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /FD /c 

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib \
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib \
 odbccp32.lib /nologo /subsystem:windows /incremental:no \
 /pdb:$(INTDIR)/dislite.pdb /machine:I386 /out:$(INTDIR)/dislite.exe

else #  "$(CFG)" == "Debug"

INTDIR=Debug

CPP_PROJ=/nologo /G6 /MT /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /I "../nameserver/include" /I "$(PATH2BS)/nameserver/include" /Fo"$(INTDIR)/" /FD /c 

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:no\
 /pdb:$(INTDIR)/dislite.pdb /debug /machine:I386 /out:$(INTDIR)/dislite.exe

endif 

ALL : $(INTDIR)/dislite.exe

CLEAN :
	-@rm $(INTDIR)/dislite.obj
	-@rm $(INTDIR)/dislite_vc.obj
	-@rm $(INTDIR)/vc50.idb
	-@rm $(INTDIR)/dislite.exe
	-@rm $(INTDIR)/dislite.pdb

$(INTDIR) :
	$(if $(shell test -e $(INTDIR)),,-@echo "Creating $(INTDIR) directory...";mkdir $(INTDIR))

$(INTDIR)/dislite.exe : $(INTDIR) $(LINK32_OBJS)
	$(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)

################################################################################

$(INTDIR)/dislite.obj : $(INTDIR)
	$(CPP) $(CPP_PROJ) ./src/dislite.cxx

################################################################################

$(INTDIR)/dislite_vc.obj : $(INTDIR)
	$(CPP) $(CPP_PROJ) ./src/dislite_vc.cxx

################################################################################

