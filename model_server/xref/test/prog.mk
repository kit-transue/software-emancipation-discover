NAME:=xref-test

SOURCES:=\
XrefTest.cpp \
XrefTableTest.cpp \

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-I discover/model_server/xref/include \
-I discover/libs/gen/include \
-I discover/model_server/dd/include \
-I discover/model_server/obj/include \
-I discover/libs/machine/include \
-I discover/third/nihcl/include \
-I packages/cppunit/include \
-I discover/libs/gt/include \
-I discover/libs/stream_message/include \
-I discover/model_server/assoc/include \
-I discover/model_server/cmd/include \
-I discover/model_server/driver_aset/include \
-I discover/model_server/els/include \
-I discover/model_server/interface/include \
-I discover/model_server/ldr/include \
-I discover/model_server/project/include \
-I discover/model_server/project_pdf/include \
-I discover/model_server/rtl/include \
-I discover/model_server/smt/include \
-I discover/model_server/smt_metric/include \
-I discover/model_server/ste/include \
-I discover/model_server/ui/include \
-I discover/model_server/ui_misc/include \
-I discover/paracancel/include \
-I discover/model_server/machine_prefs/include \
-I discover/libs/xxinterface/include \
-I discover/model_server/api/include \
-I discover/model_server/driver_mb/include \
-I discover/model_server/process/include \

# XXX: these cannot all be required!  Notably: why the project dependency?
# ended up just grabbing everything from model_server....
DEPENDENCIES:=\
discover/model_server/api \
discover/model_server/api_pset \
discover/model_server/assoc \
discover/model_server/ast \
discover/model_server/dd \
discover/model_server/dfa \
discover/model_server/driver \
discover/model_server/driver_mb \
discover/model_server/els \
discover/model_server/init \
discover/model_server/machine_prefs \
discover/model_server/obj \
discover/model_server/process \
discover/model_server/project \
discover/model_server/project_cm \
discover/model_server/project_file \
discover/model_server/project_pdf \
discover/model_server/rtl \
discover/model_server/save \
discover/model_server/search \
discover/model_server/smt \
discover/model_server/smt_metric \
discover/model_server/to_cpp \
discover/model_server/util \
discover/model_server/xref \
discover/libs/els/if_parser \
discover/libs/els/mapper \
discover/libs/gen \
discover/libs/machine \
discover/libs/nameserver \
discover/libs/path_utils \
discover/third/nihcl \
discover/libs/stream_message \
discover/libs/xxinterface \
discover/libs/cppunit_main \
packages/cppunit \

# XXX: a better list would be:
#discover/model_server/xref \
#discover/libs/gen \
#discover/libs/machine \
#discover/third/nihcl \
#discover/libs/cppunit_main \
#packages/cppunit \

# XXX: should not require TCL
EXTRA_LDFLAGS:=\
-ltcl \

BIN_DIR:=$(TEST_BIN_DIR)


# EOF