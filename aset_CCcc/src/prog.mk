# Old Discover-adapted EDG parser

# Use mk_errinfo from Coverity build!

MK_ERRINFO:=$(TEST_BIN_DIR)/mk_errinfo

ERROR_FILES:= $(SOURCE_DIR)/../include/err_codes.h $(SOURCE_DIR)/../include/err_data.h

$(ERROR_FILES): SOURCE_DIR:=$(SOURCE_DIR)

$(ERROR_FILES): $(SOURCE_DIR)/error_msg.txt $(SOURCE_DIR)/error_tag.txt
	cd $(SOURCE_DIR); ../../../$(MK_ERRINFO) error_msg.txt error_tag.txt ../include/err_codes.h ../include/err_data.h


NAME:=aset_CCcc

SOURCES:= \
SET_CCcc_msg.cxx \
SET_additions.c \
SET_ast.c \
SET_complaints.cxx \
SET_dump_if.c \
SET_file_table.c \
SET_il.c \
SET_main.cxx \
SET_multiple_iffs.c \
SET_preprocess.c \
SET_process_entry.c \
SET_scope.cxx \
SET_symbol.c \
SET_symid.c \
attribute.c \
c_gen_be.c \
cfe.c \
class_decl.c \
cmd_line.c \
const_ints.c \
cp_gen_be.c \
debug.c \
decl_inits.c \
decl_spec.c \
declarator.c \
decls.c \
def_arg.c \
disambig.c \
error.c \
expr.c \
exprutil.c \
extasm.c \
fe_init.c \
fe_wrapup.c \
float_pt.c \
folding.c \
func_def.c \
host_envir.c \
il.c \
il_alloc.c \
il_display.c \
il_read.c \
il_to_str.c \
il_walk.c \
il_write.c \
inline.c \
layout.c \
lexical.c \
literals.c \
lookup.c \
lower_c99.c \
lower_eh.c \
lower_il.c \
lower_init.c \
lower_name.c \
macro.c \
mem_manage.c \
ms_attrib.c \
overload.c \
pch.c \
pragma.c \
preproc.c \
scope_stk.c \
src_seq.c \
statements.c \
symbol_ref.c \
symbol_tbl.c \
sys_predef.c \
target.c \
templates.c \
trans_copy.c \
trans_corresp.c \
trans_unit.c \
types.c \

DEPENDENCIES:=\
discover/libs/path_utils \
discover/libs/stream_message \
discover/libs/xxinterface \
discover/third/md5 \

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-D SOFTWARE_EMANCIPATION_PARSER \
-I discover/aset_CCcc/include \
-I discover/libs/stream_message/include \
-I discover/libs/xxinterface/include \
-I discover/third/md5/include/saved \
-I discover/third/md5/include \

EXTRA_CFLAGS:=\
-D SOFTWARE_EMANCIPATION_PARSER \
-I discover/aset_CCcc/include \
-I discover/libs/stream_message/include \
-I discover/libs/xxinterface/include \
-I discover/third/md5/include/saved \
-I discover/third/md5/include \

EXTRA_DEPENDENCIES:=$(ERROR_FILES)


# EOF