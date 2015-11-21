SOURCES:=\
aconnectn.cxx \
mbmsgser.cxx \
mbuildmsg.cxx \
minidom.cxx \
minisax.cxx \
minixml.cxx \
msg_comms.cxx \
msg_format.cxx \
msg_stdout.cxx \
service.cxx \
startprocess.cxx \
threads.cxx \
transport.cxx \
viewer_is_running.cxx \

ifeq ($(mc_platform),mingw)
THREAD_IMPLEMENTATION = WIN32_THREAD_IMPLEMENTATION
else
THREAD_IMPLEMENTATION = PTHREAD_THREAD_IMPLEMENTATION
endif

EXTRA_CXXFLAGS += \
-D ISO_CPP_HEADERS \
-D $(THREAD_IMPLEMENTATION) \
-I discover/libs/stream_message/include \
-I discover/libs/nameserver/include \

# EOF