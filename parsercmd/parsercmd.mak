PATHTOROOT=..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

EXEC=$(INTDIR)/parsercmd.exe

OBJS= \
	$(INTDIR)/clconvert.obj \
	$(INTDIR)/clinterpret.obj \
	$(INTDIR)/configpar.obj \
	$(INTDIR)/main.obj

VPATH= ./src

ALL : $(EXEC)

# Linking stuff

XERCESLIB=$(PATH2BS)/thirdparty/xerces/NT/xerces-c_1.lib

LINK32=link.exe
LINK32_FL=	kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
		advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
		odbc32.lib odbccp32.lib wsock32.lib libcpmt.lib "$(XERCESLIB)"\
		/nologo /subsystem:console /incremental:no /machine:I386\
		/pdb:"$(INTDIR)/parsercmd.pdb" /out:"$(INTDIR)/parsercmd.exe"
ifeq "$(CFG)" "Debug"
	LINK32_FLAGS=$(LINK32_FL) /debug /fixed:no
else
	LINK32_FLAGS=$(LINK32_FL)
endif

LINK32_LIBS=\
	./libs/nameserver/$(INTDIR)/nameserver.lib\
	./libs/stream_message/$(INTDIR)/stream_message.lib

# Compilation stuff

CPP_INCLUDES=\
	/I ./include \
	/I $(PATHTOROOT)/stream_message/include \
	/I $(PATH2BS)/stream_message/include

$(EXEC) : $(INTDIR) $(OBJS)
	$(LINK32) $(LINK32_FLAGS) $(OBJS) $(LINK32_LIBS)

include $(MAKESCRIPTSPATH)/targets.mak

