NAME:=stream_message

SOURCE_DIR:=discover/libs/stream_message/src

ifeq ($(mc_platform),mingw)
EXTRA_LINK_FLAGS:= -lwsock32
else
EXTRA_LINK_FLAGS:= 
endif

DEPENDENCIES:=\
discover/libs/nameserver \


# EOF