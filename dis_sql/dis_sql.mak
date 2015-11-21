PATHTOROOT=..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

MAKE=$(MAKEEXE) $(MAKEOPT)

LINK32=link.exe
CPLUSPLUS_FLAGS=
EXEC=$(INTDIR)/dis_sql.exe
INCLUDE=/I $(subst \,/,$(MSVCDir))/include /I ./include

OBJS= \
	$(INTDIR)/sybase.yacc.obj \
	$(INTDIR)/sybase.lex.obj \
	$(INTDIR)/sql.yacc.obj \
	$(INTDIR)/sql.lex.obj \
	$(INTDIR)/sql_ast.obj \
	$(INTDIR)/sql_ast_enum.obj \
	$(INTDIR)/sql_ast_map.obj \
	$(INTDIR)/sql_builtin.obj \
	$(INTDIR)/sql_driver.obj \
	$(INTDIR)/sql_iff.obj \
	$(INTDIR)/sql_keyword.obj \
	$(INTDIR)/sql_keytable.obj \
	$(INTDIR)/sql_host.obj \
	$(INTDIR)/sql_resolve.obj \
	$(INTDIR)/sql_sym_enum.obj \
	$(INTDIR)/sql_symbol.obj \
	$(INTDIR)/sym_hash.obj \
	$(INTDIR)/esql.obj \
	$(INTDIR)/esqlsym.obj

VPATH=./src

EXTRN_INCLUDE=/I $(PATHTOROOT)/include \
	/I $(PATHTOROOT)/machine/include \
	/I $(PATHTOROOT)/nihcl/include \
	/I $(PATHTOROOT)/gen/include \
	/I $(PATH2BS)/include \
	/I $(PATH2BS)/machine/include \
	/I $(PATH2BS)/nihcl/include \
	/I $(PATH2BS)/gen/include

CPP_PROJ=$(CPP_BASE)\
	/I ./include \
	/I $(INTDIR) \
	$(INCLUDE) \
	$(EXTRN_INCLUDE)\
	/D "WIN32" /D "_CONSOLE"\
	/Fo"$(INTDIR)/" /c /Tp
	

LINK32_FL=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
		advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
		odbc32.lib odbccp32.lib libcmt.lib libcimt.lib wsock32.lib\
		mpr.lib /nologo /subsystem:console /incremental:no\
		/nodefaultlib:libcimtd.lib /machine:I386\
		/pdb:$(INTDIR)/dis_sql.pdb /out:$(INTDIR)/dis_sql.exe

ifeq "$(CFG)" "Debug"
	LINK32_FLAGS=$(LINK32_FL) /debug
else
	LINK32_FLAGS=$(LINK32_FL)
endif

LINK32_LIBS=./libs/nihcl/$(INTDIR)/nihcl.lib \
	./libs/machine/$(INTDIR)/machine.lib

ALL : $(EXEC)

CLEAN : 
	-@rm $(OBJS)
	-@rm $(EXEC)

$(INTDIR) :
	$(if $(shell test -e $(INTDIR)),,-@echo "Creating $(INTDIR) directory...";mkdir $(INTDIR))
 
$(EXEC) : $(INTDIR) $(OBJS)
	$(LINK32) $(LINK32_FLAGS) $(OBJS) $(LINK32_LIBS)

################################################################################

SOURCE=./src/sybase.y

$(INTDIR)/sybase.yacc.obj : $(INTDIR) $(SOURCE)
	$(MAKE) -f $(YL_MAKE)\
		EXTRN_INCLUDE="$(EXTRN_INCLUDE) $(INCLUDE)"\
		YACC="$(ADMMAKE)/bison.exe -d -y"\
		VPATH=./src \
		INTDIR=$(INTDIR)\
		CPP="$(CPP)" \
		WORK_BASE=sybase yacch

################################################################################

SOURCE=./src/sybase.l

$(INTDIR)/sybase.lex.obj : $(INTDIR) $(SOURCE)
	$(MAKE) -f $(YL_MAKE)\
		EXTRN_INCLUDE="$(EXTRN_INCLUDE) $(INCLUDE)"\
		LEX="$(ADMMAKE)/FLEX_2.5.4.EXE\
			 -S$(ADMMAKE)/FLEX_2.5.4.SKL -i"\
		CPP_PROJ="/D YY_SKIP_YYWRAP /D __STDC__"\
		VPATH=./src\
		INTDIR=$(INTDIR)\
		CPP="$(CPP)" \
		WORK_BASE=sybase lex_targ

################################################################################

SOURCE=./src/sql.y

$(INTDIR)/sql.yacc.obj : $(INTDIR) $(SOURCE)
	$(MAKE) -f $(YL_MAKE)\
		EXTRN_INCLUDE="$(EXTRN_INCLUDE) $(INCLUDE)"\
		VPATH=./src\
		INTDIR=$(INTDIR)\
		CPP="$(CPP)" \
		WORK_BASE=sql yacch

################################################################################

SOURCE=./src/sql.l

$(INTDIR)/sql.lex.obj : $(INTDIR) $(SOURCE)
	$(MAKE) -f $(YL_MAKE)\
		EXTRN_INCLUDE="$(EXTRN_INCLUDE) $(INCLUDE)"\
		LEX="$(ADMMAKE)/FLEX_2.5.4.EXE\
			 -S$(ADMMAKE)/FLEX_2.5.4.SKL -i"\
		CPP_PROJ="/D YY_SKIP_YYWRAP /D __STDC__"\
		VPATH=./src\
		INTDIR=$(INTDIR)\
		CPP="$(CPP)" \
		WORK_BASE=sql lex_targ

################################################################################

include $(MAKESCRIPTSPATH)/suffixes.mak

