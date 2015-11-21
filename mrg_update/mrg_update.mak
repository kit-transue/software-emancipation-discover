# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

ifeq "$(CFG)" ""
CFG=Debug
$(warning No configuration specified.  Defaulting to Debug.)
endif 

ifneq "$(CFG)" "Release" 
ifneq "$(CFG)" "Debug"
$(warning Invalid configuration "$(CFG)" specified.)
$(warning You can specify a configuration when running NMAKE on this makefile)
$(warning by defining the macro CFG on the command line.  For example:)
$(warning )
$(warning NMAKE /f "mrg_update.mak" CFG="Debug")
$(warning )
$(warning Possible choices for configuration are:)
$(warning )
$(warning "Release" (based on "Win32 (x86) Console Application"))
$(warning "Debug" (based on "Win32 (x86) Console Application"))
$(warning )
$(error An invalid configuration is specified.)
endif
endif 

ifeq  "$(CFG)" "Release"

INTDIR=Release

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib msvcrt.lib wsock32.lib mpr.lib /nologo /subsystem:console\
 /incremental:no /pdb:$(INTDIR)/mrg_update.pdb /machine:I386\
 /out:$(INTDIR)/mrg_update.exe 

else # "$(CFG)" == "Debug"

INTDIR=Debug

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib msvcrt.lib wsock32.lib mpr.lib /nologo /subsystem:console\
 /incremental:no /pdb:$(INTDIR)/mrg_update.pdb /debug /machine:I386\
 /out:$(INTDIR)/mrg_update.exe 

endif 

ALL : $(INTDIR)/mrg_update.exe

CLEAN : 
	-@rm $(INTDIR)/mrg_update.exe
	-@rm $(INTDIR)/mrg_update.pdb

$(INTDIR) :
	$(if $(shell test -e $(INTDIR)),,-@echo "Creating $(INTDIR) directory...";mkdir $(INTDIR))

LINK32=link.exe
LINK32_LIBS=./libs/nihcl/$(INTDIR)/nihcl.lib \
	./libs/machine/$(INTDIR)/machine.lib \
	./libs/stream_message/$(INTDIR)/stream_message.lib \
	./libs/nameserver/$(INTDIR)/nameserver.lib \
	./libs/gen/$(INTDIR)/gen.lib \
	./libs/update/$(INTDIR)/update.lib 

$(INTDIR)/mrg_update.exe : $(INTDIR) $(DEF_FILE) $(LINK32_LIBS)
	$(LINK32) $(LINK32_FLAGS) $(LINK32_LIBS)

