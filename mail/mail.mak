# Microsoft Developer Studio Generated NMAKE File, Based on dislite.dsp
ifeq "$(CFG)" ""
CFG=Debug
$(warning No configuration specified. Defaulting to dislite_-_Win32_Debug.)
endif 

ifneq "$(CFG)" "mail_-_Win32_Release" 
ifneq "$(CFG)" "mail_-_Win32_Debug"
$(warning Invalid configuration "$(CFG)" specified.)
$(warning You can specify a configuration when running NMAKE)
$(warning by defining the macro CFG on the command line. For example:)
$(warning )
$(warning NMAKE /f "dislite.mak" CFG="mail_-_Win32_Debug")
$(warning )
$(warning Possible choices for configuration are:)
$(warning )
$(warning "mail_-_Win32_Release" (based on "Win32 (x86) Application"))
$(warning "mail_-_Win32_Debug" (based on "Win32 (x86) Application"))
$(warning )
$(error An invalid configuration is specified.)
endif
endif 

CPP=cl.exe
LINK32=link.exe

ifeq  "$(CFG)" "mail_-_Win32_Release"

INTDIR=Release
OUTDIR=Release

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D\
 "_MBCS" /I ./include /Fp$(INTDIR)/mail.pch /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /FD\
 /c 

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib /nologo\
 /subsystem:console /incremental:no /pdb:$(OUTDIR)/mail.pdb /machine:I386\
 /out:$(OUTDIR)/mail.exe

else #  "$(CFG)" == "mail_-_Win32_Debug"

INTDIR=Debug
OUTDIR=Debug

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /D "_MBCS" /I ./include /Fp"$(INTDIR)/mail.pch" /YX /Fo"$(INTDIR)/" \
 /c 

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib /nologo\
 /subsystem:console /incremental:no /pdb:$(OUTDIR)/mail.pdb /debug\
 /machine:I386 /out:$(OUTDIR)/mail.exe /pdbtype:sept 

endif 

LINK32_OBJS= \
	$(INTDIR)/mail.obj

ALL : $(INTDIR)/mail.exe

CLEAN :
	-@rm $(INTDIR)/mail.obj
	-@rm $(INTDIR)/mail.exe
	-@rm $(INTDIR)/mail.pdb

$(INTDIR) :
	$(if $(shell test -e $(INTDIR)),,-@echo "Creating $(INTDIR) directory...";mkdir $(INTDIR))

$(INTDIR)/mail.exe : $(INTDIR) $(LINK32_OBJS)
	$(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)

################################################################################

$(INTDIR)/mail.obj : $(INTDIR)
	$(CPP) $(CPP_PROJ) ./src/mail.cpp

################################################################################

