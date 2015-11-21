# Simplify.h

NAME:=simplify

SOURCES:= \
InclusionNode.cxx \
InclusionTree.cxx \
ModifySrc.cxx \
ParserCallBacks.cxx \
cleanup.cxx \
main.cxx \
simplify.cxx \
simplify_CBS.cxx \
simplify_globals.cxx \
stubs.cxx \

DEPENDENCIES:=\
discover/libs/els/if_parser \
discover/libs/machine \
discover/libs/xxinterface \
discover/third/gala/extern/ads \
discover/third/gala/extern/osport \
#discover/libs/source \

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-I discover/simplify/include \
-I discover/libs/machine/include \
-I discover/libs/els/if_parser/include \
-I discover/third/gala/extern/ads/include \
-I discover/third/gala/extern/osport/include \

EXTRA_CFLAGS:=\



# EOF