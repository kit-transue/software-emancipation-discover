# Microsoft Developer Studio Generated NMAKE File, Based on scandsp.dsp
ifeq "$(CFG)" ""
CFG=scandsp - Win32 Debug
$(warning No configuration specified. Defaulting to scandsp - Win32 Debug.)
endif 

ifneq "$(CFG)" "scandsp - Win32 Release" 
ifneq "$(CFG)" "scandsp - Win32 Debug"
$(warning Invalid configuration "$(CFG)" specified.)
$(warning You can specify a configuration when running NMAKE)
$(warning by defining the macro CFG on the command line. For example:)
$(warning )
$(warning NMAKE /f "scandsp.mak" CFG="scandsp - Win32 Debug")
$(warning )
$(warning Possible choices for configuration are:)
$(warning )
$(warning "scandsp - Win32 Release" (based on "Win32 (x86) Console Application"))
$(warning "scandsp - Win32 Debug" (based on "Win32 (x86) Console Application"))
$(warning )
$(error An invalid configuration is specified.)
endif 
endif 

CPP=cl.exe
LINK32=link.exe
VPATH=./src

ifeq  "$(CFG)" "scandsp - Win32 Release"

OUTDIR=Release
INTDIR=Release

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ./include /I ../stream_message/include /I $(PATH2BS)/stream_message/include\
	/D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "ISO_CPP_HEADERS" \
	/Fp$(INTDIR)/scandsp.pch /YX /Fo"$(INTDIR)/" /c 

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib \
	shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib \
	user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib \
	oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:console \
	/incremental:no /pdb:$(OUTDIR)/scandsp.pdb /machine:I386 /out:$(OUTDIR)/scandsp.exe 

else #  "$(CFG)" == "scandsp - Win32 Debug"

OUTDIR=Debug
INTDIR=Debug

CPP_PROJ=/nologo /MDd /W3 /GX /ZI /Od /I ./include /I ../stream_message/include /I $(PATH2BS)/stream_message/include\
	/D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "ISO_CPP_HEADERS" \
	/Fp$(INTDIR)/scandsp.pch /YX /Fo"$(INTDIR)/" /GZ /c 

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib \
	shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib \
	user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib \
	oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:console \
	/incremental:yes /pdb:$(OUTDIR)/scandsp.pdb /debug /machine:I386 \
	/out:$(OUTDIR)/scandsp.exe /pdbtype:sept 

endif 

ALL : $(OUTDIR)/scandsp.exe

LINK32_OBJS= \
	$(OUTDIR)/line.obj \
	$(OUTDIR)/main.obj \
	$(OUTDIR)/nametracker.obj \
	$(OUTDIR)/optparser.obj \
	$(OUTDIR)/parseline.obj \
	$(INTDIR)/registry_if.obj \
	$(INTDIR)/token.obj \
	$(INTDIR)/parser.obj \
	$(INTDIR)/circlebuf.obj \
	$(INTDIR)/dspparser.obj \
	$(INTDIR)/dswparser.obj \
	$(INTDIR)/entity.obj \
	./libs/nameserver/$(INTDIR)/nameserver.lib\
	./libs/stream_message/$(INTDIR)/stream_message.lib

$(OUTDIR)/scandsp.exe : $(OUTDIR) $(LINK32_OBJS)
	$(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)

PATHTOROOT=..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/targets.mak
