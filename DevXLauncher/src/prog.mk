NAME:=DevXLauncher

SOURCES:= \
DevXLauncher.cpp \
#DevTCLauncher.cpp \

DEPENDENCIES:=\
discover/libs/nameserver \
discover/libs/StartExe \

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-I discover/DevXLauncher/include \
-I discover/libs/nameserver/include \

EXTRA_CFLAGS:=\

#EXTRA_LDFLAGS:=\
-ltcl \


# EOF