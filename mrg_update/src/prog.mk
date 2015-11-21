NAME:=mrg_update

SOURCES:= \
GAT_update.cxx \
hash.cxx \
mrg_main.cxx \
mrg_util.cxx \
#memman.cxx \

DEPENDENCIES:=\
discover/libs/gen \
discover/libs/machine \
discover/model_server/search \
discover/third/nihcl \

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-I discover/mrg_update/include \
-I discover/libs/stream_message/include \
-I discover/libs/gen/include \
-I discover/libs/machine/include \
-I discover/third/nihcl/include \

EXTRA_CFLAGS:=\

EXTRA_LDFLAGS:=\
#-ltcl \


# EOF