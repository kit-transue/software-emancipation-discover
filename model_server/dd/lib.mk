NAME=dd

SOURCE_DIR:=discover/model_server/dd/src

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-D STATIC_BUILD \
-I $(SOURCE_DIR)/../include \
-I discover/model_server/driver_aset/include \
-I discover/model_server/obj/include \
-I discover/model_server/smt/include \
-I discover/model_server/xref/include \
-I discover/libs/gen/include \
-I discover/libs/machine/include \
-I discover/third/nihcl/include \
-I discover/model_server/project/include \
-I discover/model_server/project_pdf/include \
-I discover/model_server/machine_prefs/include \
-I discover/model_server/rtl/include \
-I discover/model_server/ui/include \
-I discover/model_server/ast/include \
-I analysis/ast/tree \
-I discover/model_server/interface/include \
-I discover/libs/stream_message/include \
-I discover/model_server/driver_mb/include \
-I discover/model_server/save/include \
-I discover/model_server/ste/include \
-I discover/model_server/subsystem/include \
-I discover/model_server/style/include \
-I discover/model_server/ldr/include \
-I discover/model_server/ui_OODT/include \
-I discover/model_server/smt_metric/include \
-I discover/model_server/search/include \
-I discover/libs/xxinterface/include \
-I discover/model_server/dfa/include \
-I discover/model_server/process/include \
-I discover/model_server/util/include \
-I discover/model_server/api/include \

EXTRA_SHARED_CFLAGS:=\
-I $(SOURCE_DIR)/../include \

EXTRA_LINK_FLAGS:=\

DEPENDENCIES:=\
discover/libs/gen \
discover/libs/machine \
discover/third/nihcl \
discover/libs/stream_message \
discover/libs/xxinterface \

# EOF