NAME:=dish2

SOURCES:= \
SocketComm.cxx \
dish.cxx \

DEPENDENCIES:=\
discover/libs/nameserver \
discover/libs/xxinterface \

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-I discover/dish/include \
-I discover/libs/gen/include \
-I discover/libs/nameserver/include \
-I discover/libs/xxinterface/include \

EXTRA_CFLAGS:=\

# EOF