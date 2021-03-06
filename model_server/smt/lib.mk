NAME=smt

SOURCE_DIR:=discover/model_server/smt/src

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-I $(SOURCE_DIR)/../include \
-I discover/model_server/save/include \
-I discover/libs/gen/include \
-I discover/model_server/dd/include \
-I discover/model_server/obj/include \
-I discover/model_server/xref/include \
-I discover/libs/machine/include \
-I discover/model_server/driver_aset/include \
-I discover/third/nihcl/include \
-I discover/libs/stream_message/include \
-I discover/model_server/dfa/include \
-I discover/model_server/machine_prefs/include \
-I discover/model_server/project/include \
-I discover/model_server/project_pdf/include \
-I discover/model_server/rtl/include \
-I discover/model_server/smt_metric/include \
-I discover/model_server/ui/include \
-I discover/model_server/cmd/include \
-I discover/model_server/api_ui/include \
-I discover/model_server/interface/include \
-I discover/model_server/process/include \
-I discover/model_server/ste/include \
-I discover/model_server/style/include \
-I discover/model_server/ast/include \
-I discover/model_server/util/include \
-I analysis/ast/tree \
-I discover/model_server/to_cpp/include \
-I discover/model_server/api/include \
-I discover/libs/els/if_parser/include \
-I discover/model_server/ldr/include \
-I discover/model_server/ste_disstub/include \
-I discover/model_server/view/include \
-I discover/model_server/view_rtl/include \

EXTRA_SHARED_CFLAGS:=\
-I $(SOURCE_DIR)/../include \
-I discover/libs/gen/include \

EXTRA_LINK_FLAGS:=\

DEPENDENCIES:=\
discover/libs/gen \
discover/libs/machine \
discover/third/nihcl \
discover/libs/stream_message \
discover/libs/els/if_parser \

# EOF