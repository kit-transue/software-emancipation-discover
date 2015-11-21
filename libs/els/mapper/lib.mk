NAME=mapper

SOURCE_DIR:=discover/libs/els/mapper/src

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-I $(SOURCE_DIR)/../include \
-I discover/libs/els/if_parser/include \
-I discover/third/gala/extern/ads/include \
-I discover/third/gala/extern/osport/include \
-I discover/libs/machine/include \

EXTRA_SHARED_CFLAGS:=\
-I $(SOURCE_DIR)/../include \

EXTRA_LINK_FLAGS:=\

DEPENDENCIES:=\



# EOF