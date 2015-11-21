SOURCES:=\
astbodyStruct.cxx \
astnodeList.cxx \
astnodeStruct.cxx \
attributeList.cxx \
attributeStruct.cxx \
discover-interface.cxx \
ifFileCache.cxx \
locationList.cxx \
locationStruct.cxx \
maplineList.cxx \
maplineStruct.cxx \
nodeinfoStruct.cxx \
positionStruct.cxx \
relationStruct.cxx \
stringList.cxx \
symbolStruct.cxx \
transformationList.cxx \
transformationStruct.cxx \
uintList.cxx \
# if_parser_CBS.cxx \ # stubs: should not be in a library linked with model_server.
# tester.cxx \ # XXX: move to a test.mk

GEN_SOURCES:=\
discover/iff_dotly_parser.c \
discover/iff_dotly_lexer.c \

$(GENSRC_DIR)/discover/iff_dotly_parser.c: SOURCE_DIR:=$(SOURCE_DIR)

$(GENSRC_DIR)/discover/iff_dotly_lexer.c: discover/libs/els/if_parser/src/iff_dotly.l $(GENSRC_DIR)/discover/iff_dotly_parser.c $(TEST_BIN_DIR)/flex
	mkdir -p $(dir $@)
	$(TEST_BIN_DIR)/flex -Piff_dotly -o$@ $<

$(GENSRC_DIR)/discover/iff_dotly_parser.c: discover/libs/els/if_parser/src/iff_dotly.y $(TEST_BIN_DIR)/bison
	mkdir -p $(dir $@)
	-bk edit $(SOURCE_DIR)/../include/iff_dotly_parser.h
	$(TEST_BIN_DIR)/bison -d -piff_dotly -o$@ $<
	@# bison uses a platform-specific header guard, causing the file to change for no good reason. Use linux64 guard everywhere.
	cat $(GENSRC_DIR)/discover/iff_dotly_parser.h | \
	sed s/BISON_OBJS_'.*'_GENSRC_DISCOVER_IFF_DOTLY_PARSER_H/BISON_OBJS_LINUX64_GENSRC_DISCOVER_IFF_DOTLY_PARSER_H/ \
	> $(SOURCE_DIR)/../include/iff_dotly_parser.h
	rm $(GENSRC_DIR)/discover/iff_dotly_parser.h


# EOF