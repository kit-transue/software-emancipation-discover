NAME:=mrg_ui

SOURCES:= \
difffile.cxx \
diffmanager.cxx \
framework.cxx \
mtkargs.cxx \
mtkchangeitem.cxx \
mtkdiffmanager.cxx \
mtkframework.cxx \
mtkmain.cxx \
mtktextbuffer.cxx \
servercomm.cxx \
#memman.cxx \

DEPENDENCIES:=\
discover/libs/gen \
discover/libs/machine \
discover/libs/nameserver \
discover/model_server/search \
discover/third/nihcl \

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-I discover/mrg_ui/include \
-I discover/libs/machine/include \
-I discover/libs/gen/include \

EXTRA_CFLAGS:=\

EXTRA_LDFLAGS:=\
-ltcl \
-ltk \


# EOF