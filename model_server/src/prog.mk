# model_server

NAME:=model_server

SOURCES:= \
model_server.cxx \

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

EXTRA_CXXFLAGS:=\
-I discover/model_server/init \

EXTRA_CFLAGS:=\

ifeq (1,$(IS_WINDOWS))
EXTRA_LDFLAGS:=\
-L ../../Tcl/lib \
-ltcl
else
EXTRA_LDFLAGS:=\
-ltcl
endif

# There are tons of dependencies between the model server libraries; rather than
# trying to get the command-line ordering right, ask the linker to deal with them
ifeq (1,$(IS_WINDOWS))
LD_PREFLAGS:=-Wl,--start-group -L$(OBJ_DIR)/libs
LD_POSTFLAGS:=-Wl,--end-group
endif

GEN_SOURCES:=\
discover/build_version.c \

$(GENSRC_DIR)/discover/build_version.c: FORCE
	echo "char const *build_version = \"XXX_FIX_THE_VERSION\";" > $@
	date | sed 's/\(.*\)/char const \*build_date = "\1";/' >> $@

# empty rule for forcing rules
FORCE:

# EOF