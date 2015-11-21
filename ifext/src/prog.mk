NAME:=ifext

SOURCES:= \
callbacks.cxx \
ifext.cxx \
print.cxx \

GEN_SOURCES:=\
discover/ifext_parser.c \

$(GENSRC_DIR)/discover/ifext_parser.c: SOURCE_DIR:=$(SOURCE_DIR)

$(GENSRC_DIR)/discover/ifext_parser.c: discover/ifext/src/ifext.y $(TEST_BIN_DIR)/bison
	mkdir -p $(dir $@)
	-bk edit $(SOURCE_DIR)/../include/ifext_parser.h
	$(TEST_BIN_DIR)/bison -d -pifext -o$@ $<
	@# bison uses a platform-specific header guard, causing the file to change for no good reason. Use linux64 guard everywhere.
	cat $(GENSRC_DIR)/discover/ifext_parser.h | \
	sed s/BISON_OBJS_'.*'_GENSRC_DISCOVER_IFEXT_PARSER_H/BISON_OBJS_LINUX64_GENSRC_DISCOVER_IFEXT_PARSER_H/ \
	> $(SOURCE_DIR)/../include/ifext_parser.h
	rm $(GENSRC_DIR)/discover/ifext_parser.h

DEPENDENCIES:=\
discover/libs/els/if_parser \
discover/libs/gen \
discover/libs/nameserver \
discover/third/nihcl \
discover/libs/stream_message \

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-I discover/model_server/ui/include \
-I discover/libs/gen/include \
-I discover/libs/stream_message/include \
-I discover/third/nihcl/include \

EXTRA_CFLAGS:=\

EXTRA_SHARED_CFLAGS:=\
-I $(SOURCE_DIR)/../include \



# EOF