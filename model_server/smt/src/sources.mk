SOURCES:=\
get_smt_decl.cxx \
metricSMT.cxx \
smt.cxx \
smt.h.cxx \
smtScd.h.cxx \
smtScdTypes.cxx \
smt_ast.cxx \
smt_ast2.cxx \
smt_categories.cxx \
smt_cppparm.cxx \
smt_ctree.cxx \
smt_cut.cxx \
smt_db.cxx \
smt_get_ast_new.cxx \
smt_get_node_type.cxx \
smt_ifl.cxx \
smt_ifl_CBS.cxx \
smt_interface.cxx \
smt_main.cxx \
smt_map_ast.cxx \
smt_modify.cxx \
smt_replace_text.cxx \
smt_sp.cxx \
smt_system.cxx \
smt_tcons.cxx \
smt_translate.cxx \
smt_tree.cxx \
smt_txt.cxx \
smt_view.cxx \

GEN_SOURCES:=\
discover/smt_spy_lexer.c \
discover/smt_spy_parser.c \

$(OBJ_FILE_STUB).o: $(GENSRC_DIR)/discover/smt_spy_parser.c

$(GENSRC_DIR)/discover/smt_spy_parser.c: SOURCE_DIR:=$(SOURCE_DIR)

$(GENSRC_DIR)/discover/smt_spy_lexer.c: discover/model_server/smt/src/smt_spy.l $(TEST_BIN_DIR)/flex
	mkdir -p $(dir $@)
	$(TEST_BIN_DIR)/flex -l -Psmt_spy -o$@ $<

$(GENSRC_DIR)/discover/smt_spy_parser.c: discover/model_server/smt/src/smt_spy.y $(TEST_BIN_DIR)/bison
	mkdir -p $(dir $@)
	-bk edit $(SOURCE_DIR)/../include/smt_spy_parser.h
	$(TEST_BIN_DIR)/bison -d -psmt_spy -o$@ $<
	@# bison uses a platform-specific header guard, causing the file to change for no good reason. Use linux64 guard everywhere.
	cat $(GENSRC_DIR)/discover/smt_spy_parser.h | \
	sed s/BISON_OBJS_'.*'_GENSRC_DISCOVER_SMT_SPY_PARSER_H/BISON_OBJS_LINUX64_GENSRC_DISCOVER_SMT_SPY_PARSER_H/ \
	> $(SOURCE_DIR)/../include/smt_spy_parser.h
	rm $(GENSRC_DIR)/discover/smt_spy_parser.h


# EOF