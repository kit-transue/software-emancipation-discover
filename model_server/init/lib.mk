NAME=init

SOURCE_DIR:=discover/model_server/init

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-D ISO_CPP_HEADERS \
-D STATIC_BUILD \
-I discover/simplify/include \
-I discover/libs/machine/include \
-I discover/libs/els/if_parser/include \
-I discover/libs/gen/include \
-I discover/libs/stream_message/include \
-I discover/libs/xxinterface/include \
-I discover/model_server/IDE/Logger \
-I discover/model_server/api/include \
-I discover/model_server/cmd/include \
-I discover/model_server/driver_aset/include \
-I discover/model_server/driver_mb/include \
-I discover/model_server/machine_prefs/include \
-I discover/model_server/obj/include \
-I discover/model_server/process/include \
-I discover/model_server/project/include \
-I discover/model_server/project_pdf/include \
-I discover/model_server/rtl/include \
-I discover/model_server/ui/include \
-I discover/qaengine/include \
-I discover/model_server/dd/include \
-I discover/model_server/smt_metric/include \
-I discover/model_server/xref/include \
-I discover/third/nihcl/include \
-I discover/model_server/smt/include \
-I discover/libs/gt/include \
-I discover/model_server/assoc/include \
-I discover/model_server/els/include \
-I discover/model_server/interface/include \
-I discover/model_server/ldr/include \
-I discover/model_server/ste/include \
-I discover/model_server/ui_misc/include \
-I discover/paracancel/include \
-I ../../Tcl/include \

EXTRA_SHARED_CFLAGS:=\
-I $(SOURCE_DIR)/../include \

EXTRA_LINK_FLAGS:=\

DEPENDENCIES:=\

# EOF