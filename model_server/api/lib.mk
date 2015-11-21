NAME=api

SOURCE_DIR:=discover/model_server/api/src

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-D STATIC_BUILD \
-I $(SOURCE_DIR)/../include \
-I discover/libs/gen/include \
-I discover/model_server/dd/include \
-I discover/model_server/obj/include \
-I discover/model_server/project_pdf/include \
-I discover/model_server/smt_metric/include \
-I discover/model_server/xref/include \
-I discover/libs/machine/include \
-I discover/model_server/project/include \
-I discover/model_server/smt/include \
-I discover/model_server/ui/include \
-I discover/third/nihcl/include \
-I discover/model_server/machine_prefs/include \
-I discover/model_server/rtl/include \
-I discover/libs/stream_message/include \
-I discover/libs/xxinterface/include \
-I discover/model_server/driver_aset/include \
-I discover/model_server/els/include \
-I discover/model_server/cmd/include \
-I discover/model_server/subsystem/include \
-I discover/model_server/ui_browser/include \
-I discover/libs/gt/include \
-I discover/model_server/ste/include \
-I discover/libs/gtxm/include \
-I discover/libs/nameserver/include \
-I discover/model_server/process/include \
-I discover/model_server/dfa/include \
-I discover/model_server/ldr/include \
-I discover/model_server/ast/include \
-I analysis/ast/tree \
-I discover/model_server/interface/include \
-I discover/model_server/ui_viewer/include \
-I ../../Tcl/include \

EXTRA_SHARED_CFLAGS:=\
-I $(SOURCE_DIR)/../include \
-I discover/model_server/cpp/include \

EXTRA_LINK_FLAGS:=\

DEPENDENCIES:=\
discover/libs/gen \
discover/libs/machine \
discover/third/nihcl \
discover/libs/stream_message \
discover/libs/xxinterface \
discover/libs/nameserver \

# EOF