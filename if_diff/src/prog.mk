NAME:=if_diff

SOURCES:= \
idfAst.cxx \
idfDict.cxx \
idfLocation.cxx \
idfMain.cxx \
idfMatchAst.cxx \
idfMatchSmt.cxx \
idfMatchSymbol.cxx \
idfModel.cxx \
idfReport.cxx \
idfSmt.cxx \
idfSymbol.cxx \
if_diff_CBS.cxx \

DEPENDENCIES:=\
discover/libs/els/mapper \
discover/libs/els/if_parser \
discover/third/gala/extern/osport \
discover/third/gala/extern/ads \
discover/libs/gen \

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-I discover/libs/els/if_parser/include \
-I discover/libs/gen/include \
-I discover/libs/machine/include \
-I discover/third/gala/extern/ads/include \
-I discover/libs/els/mapper/include \

EXTRA_CFLAGS:=\

EXTRA_SHARED_CFLAGS:=\
-I $(SOURCE_DIR)/../include \



# EOF