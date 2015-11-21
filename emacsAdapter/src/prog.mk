NAME:=emacsAdapter

SOURCES:= \
emacsAdapter.cpp \

DEPENDENCIES:=\
discover/libs/nameserver \
discover/libs/editorinterface \

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-I $(SOURCE_DIR)/../include \
-I discover/DevXLauncher/include \
-I discover/libs/editorinterface/include \
-I discover/libs/nameserver/include \

EXTRA_CFLAGS:=\

EXTRA_LDFLAGS:=\



# EOF