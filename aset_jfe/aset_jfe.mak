PATHTOROOT=..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

include $(PATHTOROOT)/FLEXlm/flexlm/current_defs_nt
include $(PATH2BS)/FLEXlm/flexlm/current_defs_nt

LINK32=link.exe
LINK32_FL=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib \
	advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib \
	odbc32.lib comctl32.lib netapi32.lib odbccp32.lib \
	wsock32.lib libcpmt.lib /nodefaultlib:msvcrt.lib \
	/nologo /subsystem:console /incremental:no /machine:I386 /NODEFAULTLIB:LIBCD \
	/pdb:$(INTDIR)/aset_jfe.pdb /out:$(INTDIR)/aset_jfe.exe

ifeq "$(CFG)" "Debug"
	LINK32_FLAGS=$(LINK32_FL) /debug
else
	LINK32_FLAGS=$(LINK32_FL)
endif

EXEC=\
	$(INTDIR)/aset_jfe.exe

OBJS= \
	$(INTDIR)/c_gen_be.obj \
	$(INTDIR)/cil_alloc.obj \
	$(INTDIR)/cil_to_str.obj \
	$(INTDIR)/cil_util.obj \
	$(INTDIR)/class_file.obj \
	$(INTDIR)/cmd_line.obj \
	$(INTDIR)/const_fp.obj \
	$(INTDIR)/const_int.obj \
	$(INTDIR)/debug.obj \
	$(INTDIR)/decls.obj \
	$(INTDIR)/driver.obj \
	$(INTDIR)/error.obj \
	$(INTDIR)/expr.obj \
	$(INTDIR)/fe_init.obj \
	$(INTDIR)/fe_main.obj \
	$(INTDIR)/fe_util.obj \
	$(INTDIR)/fe_wrapup.obj \
	$(INTDIR)/folding.obj \
	$(INTDIR)/gen_cil.obj \
	$(INTDIR)/hash.obj \
	$(INTDIR)/host_envir.obj \
	$(INTDIR)/inflate.obj \
	$(INTDIR)/is_unicode.obj \
	$(INTDIR)/jfe.obj \
	$(INTDIR)/jil_alloc.obj \
	$(INTDIR)/jil_file.obj \
	$(INTDIR)/jil_util.obj \
	$(INTDIR)/lexical.obj \
	$(INTDIR)/mem_manage.obj \
	$(INTDIR)/parse.obj \
	$(INTDIR)/parse_expr.obj \
	$(INTDIR)/parse_stmt.obj \
	$(INTDIR)/parse_util.obj \
	$(INTDIR)/SET_checksum.obj \
	$(INTDIR)/SET_dump_il.obj \
	$(INTDIR)/SET_IF.obj \
	$(INTDIR)/SET_IF_fmt.obj \
	$(INTDIR)/SET_jfe_msg.obj \
	$(INTDIR)/SET_list.obj \
	$(INTDIR)/SET_strings.obj \
	$(INTDIR)/SET_main.obj \
	$(INTDIR)/statements.obj \
	$(INTDIR)/util.obj

LINK32_LIBS=\
	./libs/md5/$(INTDIR)/md5.lib \
	./libs/nameserver/$(INTDIR)/nameserver.lib \
	./libs/stream_message/$(INTDIR)/stream_message.lib \
	./libs/xxinterface/$(INTDIR)/xxinterface.lib \
	$(PATH2BS)/FLEXlm/flexlm/current/i86_n3/$(LMGRLIB) \
	$(PATH2BS)/FLEXlm/flexlm/current/i86_n3/lm_new.obj

CPP_INCLUDES=\
	/I ./include\
	/I $(PATHTOROOT)/md5/include\
	/I $(PATHTOROOT)/xxinterface/include\
	/I $(PATHTOROOT)/stream_message/include\
	/I $(PATH2BS)/md5/include\
	/I $(PATH2BS)/xxinterface/include\
	/I $(PATH2BS)/stream_message/include\
	/D "WIN32" /D "_CONSOLE"\
	/D "SOFTWARE_EMANCIPATION_PARSER"\
	/D "DEFAULT_USE_CLASS_FILES"\
	/D GENERATE_CIL=FALSE\
	/D BACK_END_IS_C_GEN_BE=FALSE\
	/Fo"$(INTDIR)/" /c

VPATH=./src

ALL : $(EXEC)

CLEAN : 
	-@rm $(OBJS)
	-@rm $(LIBRARY)

$(INTDIR) :
	$(if $(shell test -e $(INTDIR)),,-@echo "Creating $(INTDIR) directory...";mkdir $(INTDIR))

$(EXEC) : $(INTDIR) $(OBJS) $(LINK32_LIBS)
	-@echo "$(LINK32) $(LINK32_FLAGS) $(OBJS) $(LINK32_LIBS)"
	$(LINK32) $(LINK32_FLAGS) $(OBJS) $(LINK32_LIBS)

include $(MAKESCRIPTSPATH)/suffixes.mak

