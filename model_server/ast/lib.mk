NAME=ast

SOURCE_DIR:=discover/model_server/ast/src

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-I $(SOURCE_DIR)/../include \
-I discover/libs/gen/include \
-I discover/libs/machine/include \
-I analysis/ast/tree \
-I discover/model_server/process/include \
-I discover/model_server/smt/include \
-I discover/model_server/ui_viewer/include \

EXTRA_SHARED_CFLAGS:=\
-I $(SOURCE_DIR)/../include \
-I discover/libs/machine/include \
-I discover/libs/gen/include \
-I analysis/ast/tree \

EXTRA_LINK_FLAGS:=\

DEPENDENCIES:=\
discover/libs/gen \
discover/libs/machine \


# EOF