NAME=if_parser

SOURCE_DIR:=discover/libs/els/if_parser/src

EXTRA_CXXFLAGS:=\
-D USE_EXTERN_LIB \
-D ISO_CPP_HEADERS \
-I $(SOURCE_DIR)/../include \
-I discover/libs/els/mapper/include \
-I discover/libs/machine/include \
-I discover/third/gala/extern/ads/include \
-I discover/third/gala/extern/osport/include \

EXTRA_SHARED_CFLAGS:=\
-I $(SOURCE_DIR)/../include \

EXTRA_LINK_FLAGS:=\

DEPENDENCIES:=\
discover/third/gala/extern/ads \
discover/third/gala/extern/osport \


# EOF