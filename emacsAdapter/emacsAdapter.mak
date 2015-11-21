# Microsoft Developer Studio Generated NMAKE File, Based on emacsAdapter.dsp
ifeq "$(CFG)" ""
CFG=emacsAdapter_-_Win32_Debug
$(warning No configuration specified. Defaulting to emacsAdapter_-_Win32_Debug.)
endif 

ifneq "$(CFG)" "emacsAdapter_-_Win32_Release" 
ifneq "$(CFG)" "emacsAdapter_-_Win32_Debug"
$(warning Invalid configuration "$(CFG)" specified.)
$(warning You can specify a configuration when running NMAKE)
$(warning by defining the macro CFG on the command line. For example:)
$(warning )
$(warning NMAKE /f "emacsAdapter.mak" CFG="emacsAdapter_-_Win32_Debug")
$(warning )
$(warning Possible choices for configuration are:)
$(warning )
$(warning "emacsAdapter_-_Win32_Release" (based on "Win32 (x86) Console Application"))
$(warning "emacsAdapter_-_Win32_Debug" (based on "Win32 (x86) Console Application"))
$(warning )
$(error An invalid configuration is specified.)
endif 
endif 

CPP=cl.exe
LINK32=link.exe

ifeq  "$(CFG)" "emacsAdapter_-_Win32_Release"

OUTDIR=Release
INTDIR=Release

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ./include /I $(LOCAL_SRC_ROOT)/DevXLauncher/include /I $(BUILD_SRC_ROOT)/DevXLauncher/include  \
	/I ../nameserver/include /I $(PATH2BS)/nameserver/include \
	/I ../editorinterface/include /I $(PATH2BS)/editorinterface/include /D "WIN32" /D "NDEBUG" /D \
	"_CONSOLE" /D "_MBCS" /Fo"$(INTDIR)/" /c 

LINK32_FLAGS=advapi32.lib wsock32.lib /nologo /subsystem:console \
	/incremental:no /pdb:$(OUTDIR)/emacsAdapter.pdb /machine:I386 /out:$(OUTDIR)/emacsAdapter.exe 

else #  "$(CFG)" == "emacsAdapter_-_Win32_Debug"

OUTDIR=Debug
INTDIR=Debug
CPP_PROJ=/nologo /MD /W3 /GX /ZI /Od /I ./include /I ../nameserver/include /I $(PATH2BS)/nameserver/include\
	/I $(LOCAL_SRC_ROOT)/DevXLauncher/include /I $(BUILD_SRC_ROOT)/DevXLauncher/include  \
	/I ../editorinterface/include /I $(PATH2BS)/editorinterface/include /D "WIN32" /D "_DEBUG" /D "_CONSOLE" \
	/D "_MBCS" /FR"$(INTDIR)/" /Fo"$(INTDIR)/" /GZ /c 

LINK32_FLAGS=advapi32.lib wsock32.lib /nologo /subsystem:console /incremental:yes \
	/pdb:$(OUTDIR)/emacsAdapter.pdb /debug /machine:I386 /out:$(OUTDIR)/emacsAdapter.exe /pdbtype:sept 

endif 

ALL : $(OUTDIR)/emacsAdapter.exe


CLEAN :
	-@erase $(INTDIR)/emacsAdapter.obj
	-@erase $(INTDIR)/emacsAdapter.sbr
	-@erase $(INTDIR)/vc60.idb
	-@erase $(INTDIR)/vc60.pdb
	-@erase $(OUTDIR)/emacsAdapter.bsc
	-@erase $(OUTDIR)/emacsAdapter.exe
	-@erase $(OUTDIR)/emacsAdapter.ilk
	-@erase $(OUTDIR)/emacsAdapter.pdb

$(OUTDIR) :
	$(if $(shell test -e $(OUTDIR)),,-@echo "Creating $(OUTDIR) directory...";mkdir $(OUTDIR))

LINK32_OBJS= \
	./libs/nameserver/$(OUTDIR)/nameserver.lib	\
	./libs/editorinterface/$(OUTDIR)/editorinterface.lib	\
	$(INTDIR)/emacsAdapter.obj

$(OUTDIR)/emacsAdapter.exe : $(OUTDIR) $(DEF_FILE) $(LINK32_OBJS)
	$(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)

$(INTDIR)/emacsAdapter.obj : $(INTDIR)
	$(CPP) $(CPP_PROJ) ./src/emacsAdapter.cpp

