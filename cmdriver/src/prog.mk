NAME:=cmdriver

SOURCES:= \
CMDriver.cxx \
CmXmlStringTokenizer.cpp \

DEPENDENCIES:=\
discover/libs/nameserver \

EXTRA_CXXFLAGS:=\
-I discover/DevXLauncher/include \
-I discover/cmdriver/CmXml/include \
-I discover/libs/nameserver/include \



# EOF