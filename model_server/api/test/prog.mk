NAME:=api-test

SOURCES:=\
tclListTest.cpp \

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-I discover/model_server/api/include \
-I discover/libs/gen/include \
-I discover/libs/machine/include \
-I packages/cppunit/include \

DEPENDENCIES:=\
discover/model_server/api \
discover/libs/machine \
discover/libs/cppunit_main \
packages/cppunit \

EXTRA_LDFLAGS:=\
-ltcl \

BIN_DIR:=$(TEST_BIN_DIR)


# EOF