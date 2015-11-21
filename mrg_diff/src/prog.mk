NAME:=mrg_diff

SOURCES:= \
main.cxx \
mrgFile.cxx \
mrg_diff.cxx \
stubs.cxx \
tokenize.cxx \

DEPENDENCIES:=\
discover/libs/gen \
discover/libs/machine \
discover/model_server/search \
discover/third/nihcl \

EXTRA_CXXFLAGS:=\
-D ISO_CPP_HEADERS \
-I discover/mrg_diff/include \
-I discover/libs/gen/include \
-I discover/libs/machine/include \

EXTRA_CFLAGS:=\

EXTRA_LDFLAGS:=\

# There are circular dependencies between the libraries; rather than
# trying to get the command-line ordering right, ask the linker to deal with them
ifeq (1,$(IS_WINDOWS))
LD_PREFLAGS:=-Wl,--start-group -L$(OBJ_DIR)/libs
LD_POSTFLAGS:=-Wl,--end-group
endif

# EOF