NAME:=nameserver

SOURCE_DIR:=discover/libs/nameserver/src

ifeq ($(mc_platform),mingw)
EXTRA_LINK_FLAGS:= -lwsock32
else
EXTRA_LINK_FLAGS:=
endif

DEPENDENCIES:=\

# EOF