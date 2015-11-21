# Microsoft Developer Studio Generated NMAKE File, Based on mrg_tk.dsp
ifeq "$(CFG)" ""
CFG=Debug
$(warning No configuration specified. Defaulting to Debug.)
endif 

ifneq "$(CFG)" "Release" 
ifneq "$(CFG)" "Debug"
$(warning Invalid configuration "$(CFG)" specified.)
$(warning You can specify a configuration when running NMAKE on this makefile)
$(warning by defining the macro CFG on the command line. For example:)
$(warning )
$(warning NMAKE /f "mrg_ui.mak" CFG="Debug")
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
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib msvcrt.lib\
 wsock32.lib mpr.lib /nologo /subsystem:windows /incremental:no\
 /pdb:$(INTDIR)/mrg_ui.pdb /machine:I386 /out:$(INTDIR)/mrg_ui.exe 

else #  "$(CFG)" == "Debug"

INTDIR=Debug

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib msvcrt.lib\
 wsock32.lib mpr.lib /nologo /subsystem:windows /incremental:no\
 /pdb:$(INTDIR)/mrg_ui.pdb /debug /machine:I386 /out:$(INTDIR)/mrg_ui.exe 

endif 

ALL : $(INTDIR)/mrg_ui.exe

CLEAN :
	-@rm $(INTDIR)/mrg_ui.exe
	-@rm $(INTDIR)/mrg_ui.pdb

$(INTDIR) :
	$(if $(shell test -e $(INTDIR)),,-@echo "Creating $(INTDIR) directory...";mkdir $(INTDIR))

LINK32=link.exe
LINK32_OBJS= \
	$(PATH2BS)/dis_tcl8.3/win/Release/tcl83.lib \
	$(PATH2BS)/dis_tk8.3/win/Release/tk83.lib \
	./libs/nameserver/$(INTDIR)/nameserver.lib \
	./libs/mtk/$(INTDIR)/mtk.lib \
	./libs/machine/$(INTDIR)/machine.lib \
	./libs/gen/$(INTDIR)/gen.lib \
	./libs/stream_message/$(INTDIR)/stream_message.lib 

$(INTDIR)/mrg_ui.exe : $(INTDIR) $(LINK32_OBJS)
	$(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)
	
