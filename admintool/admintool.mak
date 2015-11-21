# Microsoft Developer Studio Generated NMAKE File, Based on admintool.dsp
ifeq "$(CFG)" ""
CFG=admintool_-_Win32_Debug
$(warning No configuration specified. Defaulting to admintool_-_Win32_Debug.)
endif 

ifneq "$(CFG)" "admintool_-_Win32_Release" 
ifneq "$(CFG)" "admintool_-_Win32_Debug"
$(warning Invalid configuration "$(CFG)" specified.)
$(warning You can specify a configuration when running NMAKE)
$(warning by defining the macro CFG on the command line. For example:)
$(warning )
$(warning NMAKE /f "admintool.mak" CFG="admintool_-_Win32_Debug")
$(warning )
$(warning Possible choices for configuration are:)
$(warning )
$(warning "admintool_-_Win32_Release" (based on "Win32 (x86) Console Application"))
$(warning "admintool_-_Win32_Debug" (based on "Win32 (x86) Console Application"))
$(warning )
$(error An invalid configuration is specified.)
endif 
endif 

CPP=cl.exe
LINK32=link.exe

ifeq  "$(CFG)" "admintool_-_Win32_Release"

OUTDIR=Release
INTDIR=Release

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D\
 "_MBCS" /I ../nameserver/include /I ../dish/include /I $(PATH2BS)/nameserver/include /I $(PATH2BS)/dish/include /Fo"$(INTDIR)/" /c 

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib /nologo\
 /subsystem:console /incremental:no /pdb:$(OUTDIR)/admintool.pdb /machine:I386\
 /out:$(OUTDIR)/admintool.exe 

else #  "$(CFG)" == "admintool_-_Win32_Debug"

OUTDIR=Debug
INTDIR=Debug

CPP_PROJ=/nologo /MLd /W3 /GX /Z7 /Od /Ob1 /D "WIN32" /D "_DEBUG" /D\
 "_CONSOLE" /D "_MBCS" /I ../nameserver/include /I ../dish/include /I $(PATH2BS)/nameserver/include /I $(PATH2BS)/dish/include /Fo"$(INTDIR)/" /c 

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib /nologo\
 /subsystem:console /incremental:yes /pdb:$(OUTDIR)/admintool.pdb /debug\
 /machine:I386 /out:$(OUTDIR)/admintool.exe /pdbtype:sept 

endif 

ALL : $(OUTDIR)/admintool.exe

CLEAN :
	-@erase $(INTDIR)/admintool.obj
	-@erase $(OUTDIR)/admintool.exe
	-@erase $(OUTDIR)/admintool.ilk
	-@erase $(OUTDIR)/admintool.pdb

$(OUTDIR) :
	$(if $(shell test -e $(OUTDIR)),,-@echo "Creating $(OUTDIR) directory...";mkdir $(OUTDIR))
 
LINK32_OBJS=$(INTDIR)/admintool.obj \
            ./libs/nameserver/$(INTDIR)/nameserver.lib \
            ./libs/dish/$(INTDIR)/dish.lib

$(OUTDIR)/admintool.exe : $(OUTDIR) $(LINK32_OBJS)
	$(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)

$(INTDIR)/admintool.obj : $(SOURCE) $(INTDIR)
	$(CPP) $(CPP_PROJ) ./src/admintool.cpp

