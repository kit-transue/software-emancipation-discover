# Microsoft Developer Studio Generated NMAKE File
ifeq "$(CFG)" ""
CFG=ci_environment - Win32 Debug
$(warning No configuration specified. Defaulting to ci_environment - Win32 Debug.)
endif 

ifneq "$(CFG)" "ci_environment - Win32 Release" 
ifneq "$(CFG)" "ci_environment - Win32 Debug"
$(warning Invalid configuration "$(CFG)" specified.)
$(warning You can specify a configuration when running NMAKE)
$(warning by defining the macro CFG on the command line. For example:)
$(warning )
$(warning NMAKE /f "ci_environment.mak" CFG="ci_environment - Win32 Debug")
$(warning )
$(warning Possible choices for configuration are:)
$(warning )
$(warning "ci_environment - Win32 Release" (based on "Win32 (x86) Console Application"))
$(warning "ci_environment - Win32 Debug" (based on "Win32 (x86) Console Application"))
$(warning )
$(error An invalid configuration is specified.)
endif 
endif 

CPP=cl.exe
LINK32=link.exe

ifeq  "$(CFG)" "ci_environment - Win32 Release"

OUTDIR=Release
INTDIR=Release

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ./include /I ../machine/include /I $(PATH2BS)/machine/include /I ../scandsp/include /I $(PATH2BS)/scandsp/include\
	/D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "ISO_CPP_HEADERS" \
	/Fo"$(INTDIR)/" /c 

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib \
	advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib \
	odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib \
	shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib mpr.lib \
	/nologo /subsystem:console /incremental:no /pdb:$(OUTDIR)/ci_environment.pdb \
	/machine:I386 /out:$(OUTDIR)/ci_environment.exe 

else #  "$(CFG)" == "ci_environment - Win32 Debug"

OUTDIR=Debug
INTDIR=Debug

CPP_PROJ=/nologo /MD /W3 /GX /ZI /Od /I ./include /I ../machine/include /I $(PATH2BS)/machine/include /I ../scandsp/include /I $(PATH2BS)/scandsp/include \
	/D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "ISO_CPP_HEADERS" \
	/Fo"$(INTDIR)/" /GZ /c 

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib \
	shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib \
	user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib \
	oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib mpr.lib /nologo /subsystem:console \
	/incremental:yes /pdb:$(OUTDIR)/ci_environment.pdb /debug /machine:I386 \
	/out:$(OUTDIR)/ci_environment.exe /pdbtype:sept 

endif 

ALL : $(OUTDIR)/ci_environment.exe


CLEAN :
	-@rm $(INTDIR)/ci_environment.obj
	-@rm $(INTDIR)/node.obj
	-@rm $(INTDIR)/pathname.obj
	-@rm $(INTDIR)/pdf.obj
	-@rm $(INTDIR)/toplev_proj.obj
	-@rm $(INTDIR)/utils.obj
	-@rm $(INTDIR)/vc60.idb
	-@rm $(INTDIR)/vc60.pdb
	-@rm $(OUTDIR)/ci_environment.exe
	-@rm $(OUTDIR)/ci_environment.ilk
	-@rm $(OUTDIR)/ci_environment.pdb

$(OUTDIR) :
	$(if $(shell test -e $(OUTDIR)),,-@echo "Creating $(OUTDIR) directory...";mkdir $(OUTDIR))

LINK32_OBJS= \
	$(INTDIR)/env_lib.obj \
	$(INTDIR)/ci_environment.obj \
	./libs/machine/$(INTDIR)/machine.lib \
	./libs/scandsp/$(INTDIR)/scandsp.lib	

$(OUTDIR)/ci_environment.exe : $(OUTDIR) $(DEF_FILE) $(LINK32_OBJS)
	$(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)

$(INTDIR)/ci_environment.obj : $(INTDIR) ./src/ci_environment.cxx
	$(CPP) $(CPP_PROJ) ./src/ci_environment.cxx

$(INTDIR)/env_lib.obj : $(INTDIR) ./src/env_lib.cxx
	$(CPP) $(CPP_PROJ) ./src/env_lib.cxx

