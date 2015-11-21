SOURCES:=\
path.h.cxx \
path0.h.cxx \
path1.h.cxx \
pathcheck.h.cxx \
patherr.h.cxx \
pathor.h.cxx \
pdf_debug.cxx \
pdf_flat.cxx \
pdf_parser.cxx \
pdf_scope.h.cxx \
pdf_selector.h.cxx \
pdf_tree.h.cxx \
projMap.h.cxx \
proj_hash.h.cxx \
scopeClone.h.cxx \
scopeLeaf.h.cxx \
scopeMaster.h.cxx \
scopeMgr.h.cxx \
scopeNode.h.cxx \
scopeRoot.h.cxx \
scopeUnit.h.cxx \
scope_cli.cxx \
util.cxx \
verify_pdf.cxx \

GEN_SOURCES:=\
discover/pdf_load_lexer.c \
discover/path_lexer.c \
discover/path_parser.c \

$(GENSRC_DIR)/discover/path_parser.c: SOURCE_DIR:=$(SOURCE_DIR)

$(GENSRC_DIR)/discover/pdf_load_lexer.c: discover/model_server/project_pdf/src/pdf_load.l $(TEST_BIN_DIR)/flex
	mkdir -p $(dir $@)
	$(TEST_BIN_DIR)/flex -l -Ppdf_load -o$@ $<

$(GENSRC_DIR)/discover/path_lexer.c: discover/model_server/project_pdf/src/path.l $(TEST_BIN_DIR)/flex $(GENSRC_DIR)/discover/path_parser.c
	mkdir -p $(dir $@)
	$(TEST_BIN_DIR)/flex -l -Ppath -o$@ $<

$(GENSRC_DIR)/discover/path_parser.c: discover/model_server/project_pdf/src/path.y $(TEST_BIN_DIR)/bison
	mkdir -p $(dir $@)
	-bk edit $(SOURCE_DIR)/../include/path_parser.h
	$(TEST_BIN_DIR)/bison -d -ppath -o$@ $<
	@# bison uses a platform-specific header guard, causing the file to change for no good reason. Use linux64 guard everywhere.
	cat $(GENSRC_DIR)/discover/path_parser.h | \
	sed s/BISON_OBJS_'.*'_GENSRC_DISCOVER_PATH_PARSER_H/BISON_OBJS_LINUX64_GENSRC_DISCOVER_PATH_PARSER_H/ \
	> $(SOURCE_DIR)/../include/path_parser.h
	rm $(GENSRC_DIR)/discover/path_parser.h


# EOF