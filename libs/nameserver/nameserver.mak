# Microsoft Developer Studio Generated NMAKE File, Based on nameserv.dsp
ifeq "$(CFG)" ""
CFG=nameserver_-_Win32_Debug
$(warning No configuration specified. Defaulting to nameserver_-_Win32_Debug)
endif 

CPP=cl.exe
LINK32=link.exe
BUILDMSG=message
VPATH=src

ifneq "$(CFG)" "nameserver_-_Win32_Release"  
ifneq "$(CFG)" "nameserver_-_Win32_Debug"
$(warning Invalid configuration "$(CFG)" specified.)
$(warning You can specify a configuration when running NMAKE)
$(warning by defining the macro CFG on the command line. For example:)
$(warning )
$(warning NMAKE /f "nameserv.mak" CFG="nameserver_-_Win32_Debug")
$(warning )
$(warning Possible choices for configuration are:)
$(warning )
$(warning "nameserver_-_Win32_Release" (based on "Win32 (x86) Console Application"))
$(warning "nameserver_-_Win32_Debug" (based on "Win32 (x86) Console Application"))
$(warning )
$(error An invalid configuration is specified.)
endif
endif 

INCLUDES=/I ./include

ifeq  "$(CFG)" "nameserver_-_Win32_Release"

OUTDIR=Release
INTDIR=Release

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D\
		"_MBCS" /Fo"$(INTDIR)/" /c $(INCLUDES)

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 		advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib /nologo\
 		/subsystem:windows /incremental:no /pdb:$(OUTDIR)/nameserv.pdb /machine:I386\
 		/out:$(OUTDIR)/nameserv.exe 

else # debug version 

OUTDIR=Debug
INTDIR=Debug

CPP_PROJ=/nologo /MLd /W3 /GX /Z7 /Od /Ob1 /D "WIN32" /D "_DEBUG" /D\
		"_CONSOLE" /D "_MBCS" /Fo"$(INTDIR)/" /c $(INCLUDES)

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 		advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib /nologo\
 		/subsystem:windows /incremental:yes /pdb:$(OUTDIR)/nameserv.pdb /debug\
 		/machine:I386 /out:$(OUTDIR)/nameserv.exe /pdbtype:sept 

endif 

LINK32_OBJS=$(INTDIR)/winmain.obj $(INTDIR)/nameServ.obj

$(OUTDIR) :
	$(if $(shell test -e $(OUTDIR)),,-@echo "Creating $(OUTDIR) directory...";mkdir $(OUTDIR))

CLEAN :
	-@echo "Cleaning..."
	-@rm $(INTDIR)/nameServ.obj
	-@rm $(INTDIR)/winmain.obj
	-@rm $(OUTDIR)/nameserv.exe
	-@rm $(OUTDIR)/nameserv.ilk
	-@rm $(OUTDIR)/nameserv.pdb

ALL : $(BUILDMSG) $(OUTDIR)/nameserv.exe

$(BUILDMSG) :
	-@echo "Building all..."

$(OUTDIR)/nameserv.exe : $(OUTDIR) $(LINK32_OBJS)
	$(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)

$(INTDIR)/%.obj:%.cxx
	$(CPP) $(CPP_PROJ) $<

