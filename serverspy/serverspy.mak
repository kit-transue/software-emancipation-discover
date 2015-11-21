# Microsoft Developer Studio Generated NMAKE File, Based on serverspy.dsp
ifeq "$(CFG)" ""
CFG=serverspy_-_Win32_Debug
$(warning No configuration specified. Defaulting to serverspy_-_Win32_Debug.)
endif 

ifneq "$(CFG)" "serverspy_-_Win32_Release" 
ifneq "$(CFG)" "serverspy_-_Win32_Debug"
$(warning Invalid configuration "$(CFG)" specified.)
$(warning You can specify a configuration when running NMAKE)
$(warning by defining the macro CFG on the command line. For example:)
$(warning )
$(warning NMAKE /f "serverspy.mak" CFG="serverspy_-_Win32_Debug")
$(warning )
$(warning Possible choices for configuration are:)
$(warning )
$(warning "serverspy_-_Win32_Release" (based on "Win32 (x86) Console Application"))
$(warning "serverspy_-_Win32_Debug" (based on "Win32 (x86) Console Application"))
$(warning )
$(error An invalid configuration is specified.)
endif 
endif 

CPP=cl.exe
LINK32=link.exe

ifeq  "$(CFG)" "serverspy_-_Win32_Release"

OUTDIR=Release
INTDIR=Release

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D\
 "_MBCS" /D "ISO_CPP_HEADERS" /D "DIRECT_MSG" /I. /I ../nameserver/include /I ../dish/include /I ../stream_message/include /I $(PATH2BS)/nameserver/include /I $(PATH2BS)/dish/include /I $(PATH2BS)/stream_message/include /Fo"$(INTDIR)/" /c 

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib /nologo\
 /subsystem:console /incremental:no /pdb:$(OUTDIR)/serverspy.pdb /machine:I386\
 /out:$(OUTDIR)/serverspy.exe 

else #  "$(CFG)" == "serverspy_-_Win32_Debug"

OUTDIR=Debug
INTDIR=Debug

CPP_PROJ=/nologo /MLd /W3 /GX /Z7 /Od /Ob1 /D "WIN32" /D "_DEBUG" /D\
 "_CONSOLE" /D "_MBCS" /D "ISO_CPP_HEADERS" /D "DIRECT_MSG" /I ../nameserver/include /I ../dish/include /I ../stream_message/include /I $(PATH2BS)/nameserver/include /I $(PATH2BS)/dish/include /I $(PATH2BS)/stream_message/include /Fo"$(INTDIR)/" /c 

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib /nologo\
 /subsystem:console /incremental:no /pdb:$(OUTDIR)/serverspy.pdb /debug\
 /machine:I386 /out:$(OUTDIR)/serverspy.exe /pdbtype:sept 

endif 

LINK32_OBJS=$(INTDIR)/serverspy.obj \
            $(INTDIR)/nameServCalls.obj \
            $(INTDIR)/socketComm.obj \
            $(INTDIR)/startprocess.obj

ALL : $(OUTDIR)/serverspy.exe

CLEAN :
	-@erase $(INTDIR)/serverspy.obj
	-@erase $(INTDIR)/nameServCalls.obj
	-@erase $(INTDIR)/socketComm.obj
	-@erase $(INTDIR)/startprocess.obj
	-@erase $(OUTDIR)/serverspy.exe
	-@erase $(OUTDIR)/serverspy.pdb

$(OUTDIR) :
	$(if $(shell test -e $(OUTDIR)),,-@echo "Creating $(OUTDIR) directory...";mkdir $(OUTDIR))

$(OUTDIR)/serverspy.exe : $(OUTDIR) $(LINK32_OBJS)
	$(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)

$(INTDIR)/serverspy.obj : $(INTDIR)
	$(CPP) $(CPP_PROJ) ./src/serverspy.cxx

$(INTDIR)/nameServCalls.obj : $(INTDIR)
	$(CPP) $(CPP_PROJ) $(PATH2BS)/nameserver/src/nameServCalls.cxx

$(INTDIR)/socketComm.obj : $(SOURCE) $(INTDIR)
	$(CPP) $(CPP_PROJ) $(PATH2BS)/dish/src/SocketComm.cxx

$(INTDIR)/startprocess.obj : $(SOURCE) $(INTDIR)
	$(CPP) $(CPP_PROJ) $(PATH2BS)/stream_message/src/startprocess.cxx

