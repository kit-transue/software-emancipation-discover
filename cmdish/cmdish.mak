# Microsoft Developer Studio Generated NMAKE File, Based on cmdish.dsp
ifeq "$(CFG)" ""
CFG=cmdish - Win32 Debug
$(warning No configuration specified. Defaulting to cmdish - Win32 Debug.)
endif 

ifneq "$(CFG)" "cmdish - Win32 Release" 
ifneq "$(CFG)" "cmdish - Win32 Debug"
$(warning Invalid configuration "$(CFG)" specified.)
$(warning You can specify a configuration when running NMAKE)
$(warning by defining the macro CFG on the command line. For example:)
$(warning )
$(warning NMAKE /f "cmdish.mak" CFG="cmdish - Win32 Debug")
$(warning )
$(warning Possible choices for configuration are:)
$(warning )
$(warning "cmdish - Win32 Release" (based on "Win32 (x86) Console Application"))
$(warning "cmdish - Win32 Debug" (based on "Win32 (x86) Console Application"))
$(warning )
$(error An invalid configuration is specified.)
endif 
endif 

CPP=cl.exe
LINK32=link.exe

ifeq  "$(CFG)" "cmdish - Win32 Release"

OUTDIR=Release
INTDIR=Release

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ../nameserver/include /I $(PATH2BS)/nameserver/include \
	/I ../DevXLauncher/include /I $(PATH2BS)/DevXLauncher/include /D "WIN32" /D "NDEBUG" /D \
	"_CONSOLE" /D "_MBCS" /Fo"$(INTDIR)/" /c 

LINK32_FLAGS=advapi32.lib wsock32.lib /nologo /subsystem:console \
	/incremental:no /pdb:$(OUTDIR)/cmdish.pdb /machine:I386 /out:$(OUTDIR)/cmdish.exe 

else #  "$(CFG)" == "cmdish - Win32 Debug"

OUTDIR=Debug
INTDIR=Debug
CPP_PROJ=/nologo /MD /W3 /GX /ZI /Od /I ../nameserver/include /I $(PATH2BS)/nameserver/include\
	/I ../DevXLauncher/include /I $(PATH2BS)/DevXLauncher/include /D "WIN32" /D "_DEBUG" /D "_CONSOLE" \
	/D "_MBCS" /FR"$(INTDIR)/" /Fo"$(INTDIR)/" /GZ /c 

LINK32_FLAGS=advapi32.lib wsock32.lib /nologo /subsystem:console /incremental:yes \
	/pdb:$(OUTDIR)/cmdish.pdb /debug /machine:I386 /out:$(OUTDIR)/cmdish.exe /pdbtype:sept 

endif 

ALL : $(OUTDIR)/cmdish.exe


CLEAN :
	-@erase $(INTDIR)/cmdish.obj
	-@erase $(INTDIR)/cmdish.sbr
	-@erase $(INTDIR)/vc60.idb
	-@erase $(INTDIR)/vc60.pdb
	-@erase $(OUTDIR)/cmdish.bsc
	-@erase $(OUTDIR)/cmdish.exe
	-@erase $(OUTDIR)/cmdish.ilk
	-@erase $(OUTDIR)/cmdish.pdb

$(OUTDIR) :
	$(if $(shell test -e $(OUTDIR)),,-@echo "Creating $(OUTDIR) directory...";mkdir $(OUTDIR))

LINK32_OBJS= \
	./libs/nameserver/$(OUTDIR)/nameserver.lib	\
	./libs/DevXLauncher/$(OUTDIR)/DevXLauncher.lib	\
	$(INTDIR)/cmdish.obj

$(OUTDIR)/cmdish.exe : $(OUTDIR) $(DEF_FILE) $(LINK32_OBJS)
	$(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)

$(INTDIR)/cmdish.obj : $(INTDIR)
	$(CPP) $(CPP_PROJ) ./src/cmdish.cxx

