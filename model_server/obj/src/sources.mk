SOURCES:=\
BooleanTree.h.cxx \
NotifySoft.cxx \
RelSentinel.h.cxx \
Rel_descrip.cxx \
SoftAssoc.h.cxx \
SoftId.h.cxx \
appMarker.h.cxx \
app_get_header.cxx \
gettime.c \
int_to_string.cxx \
objArr.h.cxx \
objArr_Int.h.cxx \
objCollection_h.cxx \
objCopy.cxx \
objDb.h.cxx \
objDictionary.cxx \
objNameTable.h.cxx \
objOp.cxx \
objOper_aux.cxx \
objOper_h.cxx \
objPropagate.h.cxx \
objRegenerator.h.cxx \
objRelation.h.cxx \
objReport.cxx \
objSet.h.cxx \
objTree.h.cxx \
obj_copy_member.cxx \
obj_mem.cxx \
obj_print.cxx \
relArr.cxx \
rel_walk.cxx \
representation.h.cxx \
string_to_int.cxx \
transaction.cxx \
tree_merge.cxx \
tree_time.cxx \

GEN_SOURCES:=\
discover/str_to_int_lexer.c \

$(GENSRC_DIR)/discover/str_to_int_lexer.c: discover/model_server/obj/src/str_to_int.l $(TEST_BIN_DIR)/flex
	mkdir -p $(dir $@)
	$(TEST_BIN_DIR)/flex -l -Pstr_to_int -o$@ $<


# EOF