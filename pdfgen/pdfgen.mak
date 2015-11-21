# Microsoft Developer Studio Generated NMAKE File, Based on pdfgen.dsp
ifeq "$(CFG)" ""
CFG=pdfgen - Win32 Debug
$(warning No configuration specified. Defaulting to pdfgen - Win32 Debug.)
endif 

ifneq "$(CFG)" "pdfgen - Win32 Release" 
ifneq "$(CFG)" "pdfgen - Win32 Debug"
$(warning Invalid configuration "$(CFG)" specified.)
$(warning You can specify a configuration when running NMAKE)
$(warning by defining the macro CFG on the command line. For example:)
$(warning )
$(warning NMAKE /f "pdfgen.mak" CFG="pdfgen - Win32 Debug")
$(warning )
$(warning Possible choices for configuration are:)
$(warning )
$(warning "pdfgen - Win32 Release" (based on "Win32 (x86) Console Application"))
$(warning "pdfgen - Win32 Debug" (based on "Win32 (x86) Console Application"))
$(warning )
$(error An invalid configuration is specified.)
endif 
endif 

CPP=cl.exe
LINK32=link.exe

ifeq  "$(CFG)" "pdfgen - Win32 Release"

OUTDIR=Release
INTDIR=Release

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ./include /I ../stream_message/include /I $(PATH2BS)/stream_message/include\
	/D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "ISO_CPP_HEADERS" \
	/Fp"$(INTDIR)/pdfgen.pch" /YX /Fo"$(INTDIR)/" /c 

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib \
	advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib \
	odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib \
	shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib \
	/nologo /subsystem:console /incremental:no /pdb:$(OUTDIR)/pdfgen.pdb \
	/machine:I386 /out:$(OUTDIR)/pdfgen.exe 

else #  "$(CFG)" == "pdfgen - Win32 Debug"

OUTDIR=Debug
INTDIR=Debug

CPP_PROJ=/nologo /MDd /W3 /GX /ZI /Od /I ./include /I ../stream_message/include /I $(PATH2BS)/stream_message/include \
	/D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "ISO_CPP_HEADERS" /Fp$(INTDIR)/pdfgen.pch \
	/YX /Fo"$(INTDIR)/" /GZ /c 

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib \
	shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib \
	user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib \
	oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:console \
	/incremental:yes /pdb:$(OUTDIR)/pdfgen.pdb /debug /machine:I386 \
	/out:$(OUTDIR)/pdfgen.exe /pdbtype:sept 

endif 

ALL : $(OUTDIR)/pdfgen.exe


CLEAN :
	-@rm $(INTDIR)/main.obj
	-@rm $(INTDIR)/node.obj
	-@rm $(INTDIR)/pathname.obj
	-@rm $(INTDIR)/pdf.obj
	-@rm $(INTDIR)/toplev_proj.obj
	-@rm $(INTDIR)/utils.obj
	-@rm $(INTDIR)/vc60.idb
	-@rm $(INTDIR)/vc60.pdb
	-@rm $(OUTDIR)/pdfgen.exe
	-@rm $(OUTDIR)/pdfgen.ilk
	-@rm $(OUTDIR)/pdfgen.pdb

$(OUTDIR) :
	$(if $(shell test -e $(OUTDIR)),,-@echo "Creating $(OUTDIR) directory...";mkdir $(OUTDIR))

LINK32_OBJS= \
	$(INTDIR)/main.obj \
	$(INTDIR)/node.obj \
	$(INTDIR)/pathname.obj \
	$(INTDIR)/pdf.obj \
	$(INTDIR)/toplev_proj.obj \
	$(INTDIR)/utils.obj \
	./libs/nameserver/$(INTDIR)/nameserver.lib \
	./libs/stream_message/$(INTDIR)/stream_message.lib	

$(OUTDIR)/pdfgen.exe : $(OUTDIR) $(DEF_FILE) $(LINK32_OBJS)
	$(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)

$(INTDIR)/main.obj : $(INTDIR)
	$(CPP) $(CPP_PROJ) ./src/main.cxx

$(INTDIR)/node.obj : $(INTDIR)
	$(CPP) $(CPP_PROJ) ./src/node.cxx

$(INTDIR)/pathname.obj : $(INTDIR)
	$(CPP) $(CPP_PROJ) ./src/pathname.cxx

$(INTDIR)/pdf.obj : $(INTDIR)
	$(CPP) $(CPP_PROJ) ./src/pdf.cxx

$(INTDIR)/toplev_proj.obj : $(INTDIR)
	$(CPP) $(CPP_PROJ) ./src/toplev_proj.cxx

$(INTDIR)/utils.obj : $(INTDIR)
	$(CPP) $(CPP_PROJ) ./src/utils.cxx


