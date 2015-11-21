NAME=driver

SOURCE_DIR:=discover/model_server/driver/src

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-D STATIC_BUILD \
-I $(SOURCE_DIR)/../include \
-I discover/libs/gen/include \
-I discover/libs/machine/include \
-I discover/libs/xxinterface/include \
-I discover/model_server/IDE/Logger \
-I discover/model_server/obj/include \
-I discover/model_server/xref/include \
-I discover/qaengine/include \
-I discover/model_server/dd/include \
-I discover/third/nihcl/include \
-I discover/libs/stream_message/include \
-I discover/model_server/driver_aset/include \
-I discover/model_server/project/include \
-I discover/model_server/project_pdf/include \
-I discover/model_server/machine_prefs/include \
-I discover/model_server/rtl/include \
-I discover/model_server/ui/include \
-I discover/model_server/api/include \
-I discover/model_server/smt_metric/include \
-I discover/model_server/smt/include \
-I ../../Tcl/include \

EXTRA_SHARED_CFLAGS:=\
-I $(SOURCE_DIR)/../include \

EXTRA_LINK_FLAGS:=\

DEPENDENCIES:=\
discover/libs/gen \
discover/libs/machine \
discover/libs/xxinterface \
discover/third/nihcl \
discover/libs/stream_message \


# EOF