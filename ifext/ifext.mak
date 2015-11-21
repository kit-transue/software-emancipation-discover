PATHTOROOT=../
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

MAKE=$(MAKEEXE) $(MAKEOPT)

LINK32=link.exe
LINK32_FL=   kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
		advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
		odbc32.lib odbccp32.lib wsock32.lib\
		/nologo /subsystem:console /incremental:no /machine:I386\
		/pdb:$(INTDIR)/ifext.pdb /out:$(INTDIR)/ifext.exe

ifeq "$(CFG)" "Debug"
	LINK32_FLAGS=$(LINK32_FL) /debug /fixed:no
else
	LINK32_FLAGS=$(LINK32_FL)
endif

EXEC=$(INTDIR)/ifext.exe

VPATH=./src

OBJS= \
	$(INTDIR)/callbacks.obj \
	$(INTDIR)/ifext.obj \
	$(INTDIR)/parser.yacc.obj \
	$(INTDIR)/print.obj

LINK32_LIBS= \
	libs/stream_message/$(INTDIR)/stream_message.lib \
	libs/nihcl/$(INTDIR)/nihcl.lib \
	libs/nameserver/$(INTDIR)/nameserver.lib \
	libs/gen/$(INTDIR)/gen.lib \
	$(PATH2BS)/extlib/NT/xerces-c_1.lib

CPP_PROJ=/MD $(CPP_OPTS)\
	$(CPP_WINDOWS)\
	/I ./include\
	/I ../model_server/ui/include\
	/I ../gen/include\
	/I ../nihcl/include\
	/I ../stream_message/include\
	/I $(PATH2BS)/model_server/ui/include\
	/I $(PATH2BS)/gen/include\
	/I $(PATH2BS)/nihcl/include\
	/I $(PATH2BS)/stream_message/include\
	$(CPP_FINISH)


ALL : $(EXEC)

CLEAN : 
	-@rm $(OBJS)
	-@rm $(EXEC)

$(INTDIR) :
	$(if $(shell test -e $(INTDIR)),,-@echo "Creating $(INTDIR) directory...";mkdir $(INTDIR))

$(EXEC) : $(INTDIR) $(OBJS)
	$(LINK32) $(LINK32_FLAGS) $(OBJS) $(LINK32_LIBS)

################################################################################

$(INTDIR)/parser.yacc.obj : $(INTDIR)
	$(MAKE) -f ../makescripts/Windows/makefile.lex \
		CPP_PROJ="/MD /D MSDOS /I include" \
		VPATH=./src \
		INTDIR=$(INTDIR)\
		CPP=cl.exe\
		WORK_BASE=parser yacch

################################################################################

include $(MAKESCRIPTSPATH)/targets.mak

