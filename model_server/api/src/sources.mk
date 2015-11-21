SOURCES:=\
AccessCatalog.cxx \
DataExchange.cxx \
GenericTree.cxx \
Interpreter.cxx \
License.cxx \
Message.cxx \
Properties.cxx \
QueryObj.cxx \
Server.cxx \
SocketClient.cxx \
TclList.cxx \
WfNode.cxx \
WfTask.cxx \
attribute.cxx \
calls.cxx \
classes.cxx \
cliCommands.cxx \
cliSmt.cxx \
cliser.cxx \
externApp.cxx \
externGroup.cxx \
extern_attr.cxx \
filter.cxx \
format.cxx \
get_sym_attr.cxx \
instance.cxx \
selection.cxx \
smtMarker.h.cxx \
smtRegion.h.cxx \
#QueryApp.cxx \
#QueryTree.cxx \
#SubChk.cxx \

GEN_SOURCES:=\
discover/range_lexer.c \
discover/filter_parser.c \

$(GENSRC_DIR)/discover/filter_parser.c: SOURCE_DIR:=$(SOURCE_DIR)

$(GENSRC_DIR)/discover/range_lexer.c: discover/model_server/api/src/range.l $(TEST_BIN_DIR)/flex
	mkdir -p $(dir $@)
	$(TEST_BIN_DIR)/flex -l -Prange -o$@ $<

$(GENSRC_DIR)/discover/filter_parser.c: discover/model_server/api/src/filter.y $(TEST_BIN_DIR)/bison
	mkdir -p $(dir $@)
	-bk edit $(SOURCE_DIR)/../include/filter_parser.h
	$(TEST_BIN_DIR)/bison -d -pfilter -o$@ $<
	@# bison uses a platform-specific header guard, causing the file to change for no good reason. Use linux64 guard everywhere.
	cat $(GENSRC_DIR)/discover/filter_parser.h | \
	sed s/BISON_OBJS_'.*'_GENSRC_DISCOVER_FILTER_PARSER_H/BISON_OBJS_LINUX64_GENSRC_DISCOVER_FILTER_PARSER_H/ \
	> $(SOURCE_DIR)/../include/filter_parser.h
	rm $(GENSRC_DIR)/discover/filter_parser.h


# EOF