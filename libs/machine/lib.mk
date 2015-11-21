NAME=machine

SOURCE_DIR:=discover/libs/machine/src

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-I $(SOURCE_DIR)/../include \
-I discover/libs/gen/include \
-I discover/libs/stream_message/include \

ifeq ($(mc_platform),mingw)
EXTRA_LINK_FLAGS:=-lmpr
else
EXTRA_LINK_FLAGS:=
endif


DEPENDENCIES:=\
discover/libs/gen \
discover/libs/stream_message \



# EOF