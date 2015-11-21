NAME:=devxAdapter

SOURCES:= \
devxAdapter.cxx

DEPENDENCIES:=\
discover/libs/nameserver \

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-I discover/libs/nameserver/include \

EXTRA_CFLAGS:=\

EXTRA_LDFLAGS:=\


# EOF