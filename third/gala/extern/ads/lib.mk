NAME=ads

SOURCE_DIR:=discover/third/gala/extern/ads/src

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-I $(SOURCE_DIR)/../include \
-I discover/libs/machine/include \
-I discover/third/gala/extern/osport/include \

EXTRA_LINK_FLAGS:=

DEPENDENCIES:=\



# EOF