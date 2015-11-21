NAME=osport

SOURCE_DIR:=discover/third/gala/extern/osport/src

ifeq ($(mc_platform),mingw)
WINDOWS_DEF = -D PCxOS
else
WINDOWS_DEF =
endif

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
$(WINDOWS_DEF) \
-I $(SOURCE_DIR)/../include \
-I discover/third/gala/extern/ads/include \

EXTRA_LINK_FLAGS:=

DEPENDENCIES:=\



# EOF