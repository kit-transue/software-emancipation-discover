NAME=project_file

SOURCE_DIR:=discover/model_server/project_file/src

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-D STATIC_BUILD \
-I $(SOURCE_DIR)/../include \
-I discover/model_server/save/include \
-I discover/libs/gen/include \
-I discover/libs/machine/include \
-I discover/libs/stream_message/include \
-I discover/model_server/cmd/include \
-I discover/model_server/driver_aset/include \
-I discover/model_server/obj/include \
-I discover/model_server/project/include \
-I discover/model_server/smt/include \
-I discover/model_server/ste/include \
-I discover/model_server/dd/include \
-I discover/model_server/ldr/include \
-I discover/model_server/machine_prefs/include \
-I discover/model_server/project_pdf/include \
-I discover/model_server/rtl/include \
-I discover/model_server/ui/include \
-I discover/model_server/xref/include \
-I discover/third/nihcl/include \
-I discover/libs/gt/include \
-I discover/model_server/api/include \
-I discover/model_server/api_ui/include \
-I discover/model_server/driver_mb/include \
-I discover/model_server/gala_communications/include \
-I discover/model_server/interface/include \
-I discover/model_server/project_cm/include \
-I discover/model_server/project_ui/include \
-I discover/model_server/util/include \
-I discover/model_server/view/include \
-I discover/model_server/smt_metric/include \
-I discover/model_server/ui_viewer/include \
-I discover/model_server \
-I discover/libs/xxinterface/include \
-I discover/model_server/dfa/include \
-I discover/model_server/ast/include \
-I analysis/ast/tree \
-I discover/model_server/IDE/configshell \
-I discover/model_server/subsystem/include \
-I ../../Tcl/include \

EXTRA_SHARED_CFLAGS:=\
-I $(SOURCE_DIR)/../include \

EXTRA_LINK_FLAGS:=\

DEPENDENCIES:=\
discover/libs/gen \
discover/libs/machine \
discover/libs/stream_message \
discover/third/nihcl \
discover/libs/xxinterface \


# EOF