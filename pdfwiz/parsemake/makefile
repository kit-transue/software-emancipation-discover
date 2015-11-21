INCLUDEFLAGS=-I. -I../messages
#CFLAGS=/c /GX /Fd /Z7 /MTd /O2
CFLAGS=/c /GX /Fd /Z7 /MTd $(OPTIMIZE)
.SUFFIXES : .obj .cxx .l .y
FLEXDRIVER=sh flexdriver

DEFAULT: parseexpr.exe

PROFILE: parseexpr.exe
	prep /lc /excall /inc dependtable_lex.obj parseexpr
	profile parseexpr
	prep /m parseexpr
	plist parseexpr.pbt

OBJECTS=\
	action.obj \
	cached_vartable.obj \
	commandline.obj \
	dependpath.obj \
	dirtree.obj \
	env_table.obj \
	file_table.obj \
	filename.obj \
	filetypes.obj \
	generate_pdf.obj \
	ide_interface.obj \
	macro_expander.obj \
	macro_node.obj \
	makefile.obj \
	make2prefs.obj \
	make_target.obj\
	make_toplevel.obj\
	mscondexpr.obj \
	mscondexpr_lex.obj \
	new_make2proj.obj \
	option_filter.obj \
	parseexpr.obj \
	pathmgr.obj \
	pdfwiz_globals.obj \
	pdfwiz_settings.obj \
	pipebuf.obj \
	prefsfile.obj \
	preprocess.obj \
	registry_if.obj \
	setech_utils.obj \
	special_macros_table.obj \
	variabletable.obj \
	vartable_chain.obj

.l.cxx:
	$(FLEXDRIVER) $*

.cxx.obj:
	$(CC) $(CFLAGS) $< /Fo$*.obj $(INCLUDEFLAGS)
	
parsemake.lib: $(OBJECTS)
	link -lib $(OBJECTS) /out:$@

parseexpr.exe: parsemake.lib main.obj
	link main.obj parsemake.lib ../messages/message.lib /map /fixed:no /debug /libpath:"C:/Program Files/DevStudio/VC/lib" advapi32.lib /out:parseexpr.exe

action.obj: action.cxx action.h cached_vartable.h env_table.h macro_node.h pathmgr.h variabletable.h

cached_vartable.obj: cached_vartable.cxx macro_node.h cached_vartable.h variabletable.h

commandline.obj: commandline.cxx commandline.h filename.h macro_node.h option_extractor.h

dependpath.obj: dependpath.cxx dependpath.h macro_expander.h make_toplevel.h pathmgr.h sort_objects.h pipebuf.h variabletable.h

dependtable_lex.obj: dependtable_lex.cxx dependpath.h

dependtable_lex.cxx: dependtable_lex.l
	$(FLEXDRIVER) dependtable_lex -PyyDepend -b -d

dirtree.obj: dirtree.cxx dirtree.h filename.h

env_table.obj: env_table.cxx cached_vartable.h env_table.h macro_node.h variabletable.h

file_table.obj : file_table.cxx file_table.h cached_vartable.h variabletable.h macro_node.h

filename.obj: filename.cxx filename.h

filetypes.obj: filetypes.cxx filetypes.h

generate_pdf.obj: generate_pdf.cxx filename.h generate_pdf.h sort_objects.h

ide_interface.obj: ide_interface.cxx ide_interface.h registry_if.h

macro_expander.obj: macro_expander.cxx macro_expander.h pipebuf.h

macro_node.obj: macro_node.cxx macro_node.h

#macrotable.obj: macrotable.cxx macro_node.h cached_vartable.h macrotable.h variabletable.h

main.obj: main.cxx macro_node.h variabletable.h dependpath.h mscondexpr.h registry_if.h prefsfile.h filename.h ide_interface.h make_toplevel.h variabletable.h

make_toplevel.obj: make_toplevel.cxx make_toplevel.h filename.h filetypes.h macro_expander.h make_target.h mscondexpr.h sort_objects.h pipebuf.h pathmgr.h pipebuf.h

make_target.obj: make_target.cxx make_target.h sort_objects.h

make2prefs.obj: make2prefs.cxx make2prefs.h generate_pdf.h

makefile.obj: makefile.cxx macro_node.h variabletable.h dependpath.h mscondexpr.h make_toplevel.h  pathmgr.h

makefile.cxx: makefile.l
	$(FLEXDRIVER) makefile -PyyMake -b -d

mscondexpr.obj: mscondexpr.cxx mscondexpr.h

mscondexpr_lex.obj: mscondexpr_lex.cxx parseexpr.obj

mscondexpr_lex.cxx: mscondexpr_lex.l
	$(FLEXDRIVER) mscondexpr_lex -PyyMSExpr

new_make2proj.obj: new_make2proj.cxx action.h cached_vartable.h commandline.h dependpath.h env_table.h filename.h filetypes.h generate_pdf.h macro_node.h make_target.h make_toplevel.h new_make2proj.cxx new_make2proj.h pathmgr.h pipebuf.h sort_objects.h special_macros_table.h variabletable.h

option_filter.obj: option_filter.cxx option_filter.h

parseexpr.obj: parseexpr.y
	bison -d parseexpr.y
	cl $(CFLAGS) parseexpr.tab.c /Foparseexpr.obj $(INCLUDEFLAGS)

pathmgr.obj: pathmgr.cxx pathmgr.h

pdfwiz_globals.obj: pdfwiz_globals.cxx action.h cached_vartable.h commandline.h dependpath.h env_table.h filename.h generate_pdf.h macro_node.h make2prefs.h make_target.h make_toplevel.h pathmgr.h pdfwiz_globals.cxx pdfwiz_globals.h pipebuf.h prefsfile.h sort_objects.h special_macros_table.h variabletable.h

pdfwiz_settings.obj: pdfwiz_settings.cxx pdfwiz_settings.cxx pdfwiz_settings.h registry_if.h setech_utils.h

pipebuf.obj: pipebuf.cxx pipebuf.h

prefsfile.obj: prefsfile.cxx prefsfile.h

preprocess.obj: preprocess.cxx

preprocess.cxx: preprocess.l
	$(FLEXDRIVER) preprocess -PyyPreprocess

read_flags_db.obj: filename.h read_flags_db.cxx

registry_if.obj: registry_if.cxx registry_if.h

special_macros_table.obj: action.h cached_vartable.h env_table.h filename.h macro_node.h make_target.h pathmgr.h sort_objects.h special_macros_table.cxx special_macros_table.h variabletable.h

setech_utils.obj: setech_utils.cxx

variabletable.obj: variabletable.cxx macro_node.h variabletable.h

vartable_chain.obj: vartable_chain.cxx cached_vartable.h env_table.h macrotable.h vartable_chain.h 

# stuff that doesn't belong in the final executable.  Test stubs, ancillary
# programs, bug investigation, proof-of-concept.

stringerr : stringerr.cxx
	cl stringerr.cxx -GX -I. "-IC:/Program Files/DevStudio/VC/include" -link /libpath:../VC/lib

test_macroexpand: test_macroexpand.cxx test_mex.l
	$(FLEXDRIVER) test_mex -PyyRecurseMacro
	cl $(CFLAGS) /Fo test_mex.cxx $(INCLUDEFLAGS)
	cl $(CFLAGS) /Fo test_macroexpand.cxx $(INCLUDEFLAGS)
	link test_mex.obj test_macroexpand.obj pipebuf.obj /map /fixed:no /debug /libpath:"C:/Program Files/DevStudio/VC/lib" libc.lib /out:test_macroexpand.exe
