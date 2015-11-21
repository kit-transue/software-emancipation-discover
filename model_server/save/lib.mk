NAME=save

SOURCE_DIR:=discover/model_server/save/src

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-I $(SOURCE_DIR)/../include \
-I discover/libs/gen/include \
-I discover/libs/machine/include \
-I discover/model_server/assoc/include \
-I discover/model_server/dd/include \
-I discover/model_server/dfa/include \
-I discover/model_server/obj/include \
-I discover/model_server/smt/include \
-I discover/model_server/ste/include \
-I discover/model_server/style/include \
-I discover/model_server/subsystem/include \
-I discover/model_server/ui/include \
-I discover/model_server/driver_aset/include \
-I discover/model_server/project/include \
-I discover/model_server/rtl/include \
-I discover/model_server/xref/include \
-I discover/third/nihcl/include \
-I discover/model_server/machine_prefs/include \
-I discover/model_server/project_pdf/include \
-I discover/libs/stream_message/include \
-I discover/model_server/interface/include \
-I discover/model_server/api/include \
-I discover/model_server/ldr/include \
-I discover/model_server/view/include \
-I discover/model_server/view_rtl/include \

EXTRA_SHARED_CFLAGS:=\
-I $(SOURCE_DIR)/../include \

EXTRA_LINK_FLAGS:=\

DEPENDENCIES:=\
discover/libs/gen \
discover/libs/machine \
discover/third/nihcl \
discover/libs/stream_message \


# EOF