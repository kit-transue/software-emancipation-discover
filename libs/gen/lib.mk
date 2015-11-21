NAME=gen

SOURCE_DIR:=discover/libs/gen/src

EXTRA_SHARED_CFLAGS:=\
-I $(SOURCE_DIR)/../include \
-I discover/libs/machine/include \
-I discover/model_server/driver_aset/include \

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-I $(SOURCE_DIR)/../include \
-I discover/libs/gt/include \
-I discover/libs/machine/include \
-I discover/libs/stream_message/include \
-I discover/third/nihcl/include \
-I discover/model_server/dd/include \
-I discover/model_server/interface/include \
-I discover/model_server/machine_prefs/include \
-I discover/model_server/obj/include \
-I discover/model_server/process/include \
-I discover/model_server/search/include \
-I discover/model_server/ui/include \

EXTRA_LINK_FLAGS:=\

DEPENDENCIES:=\
discover/libs/stream_message \
discover/libs/xxinterface \


# EOF