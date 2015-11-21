NAME:=nameserver

SOURCES:= \
nameServ.cxx \

DEPENDENCIES:=\
discover/libs/nameserver \

EXTRA_CXXFLAGS += \
-I discover/libs/nameserver/include \

# EOF