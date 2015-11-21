include $(MAKELIB)/Make.general

LOCAL_INCLUDES=\
	    -I$(LOCAL_SRC_ROOT)/emacs_integration/include \
	    -I$(LOCAL_SRC_ROOT)/nameserver/include\
	    -I$(LOCAL_SRC_ROOT)/DevXLauncher/include\
	    -I$(LOCAL_SRC_ROOT)/dish/include\
	    -I$(LOCAL_SRC_ROOT)/emacs_integration/client/include\

BUILD_INCLUDES=\
	    -I$(BUILD_SRC_ROOT)/emacs_integration/include \
	    -I$(BUILD_SRC_ROOT)/nameserver/include\
	    -I$(BUILD_SRC_ROOT)/DevXLauncher/include\
	    -I$(BUILD_SRC_ROOT)/dish/include\
	    -I$(BUILD_SRC_ROOT)/emacs_integration/client/include

all: emacsclient editorserver

editorserver: emacs_integration/server/src/server.cpp lib.o nameServCalls.o CommProto.o
	$(CCC) -o $@ $(CCARCHFLAGS) $(LOCAL_INCLUDES) $(BUILD_INCLUDES) $(CLDCTRL) -D$(ARCH) $< lib.o nameServCalls.o CommProto.o $(PRE_OBJ) $(SOCKETLIB) $(LDL) $(CXXLIBS) $(POST_OBJ)

emacsclient: emacs_integration/client/src/driver.cpp emacs_api.o lib.o startproc.o connect_mgr.o SocketComm.o nameServCalls.o CommProto.o
	$(CCC) -o $@ $(CCARCHFLAGS) $(LOCAL_INCLUDES) $(BUILD_INCLUDES) $(CLDCTRL) -D$(ARCH) $< emacs_api.o lib.o startproc.o connect_mgr.o SocketComm.o nameServCalls.o CommProto.o $(PRE_OBJ) $(SOCKETLIB) $(LDL) $(CXXLIBS) $(POST_OBJ)

emacs_api.o: emacs_integration/client/src/emacs_api.cpp
	$(CCC) -c $(CCARCHFLAGS) $(LOCAL_INCLUDES) $(BUILD_INCLUDES) -D$(ARCH) $<

lib.o: emacs_integration/src/lib.cpp
	$(CCC) -c $(CCARCHFLAGS) -D$(ARCH) $<

startproc.o: DevXLauncher/src/startproc.cpp
	$(CCC) -c $(CCARCHFLAGS) $(LOCAL_INCLUDES) $(BUILD_INCLUDES) -D$(ARCH) $<

connect_mgr.o: emacs_integration/client/src/connect_mgr.cpp
	$(CCC) -c $(CCARCHFLAGS) $(LOCAL_INCLUDES) $(BUILD_INCLUDES) -D$(ARCH) $<

SocketComm.o: dish/src/SocketComm.cxx
	$(CCC) -c $(CCARCHFLAGS) -D$(ARCH) $<

nameServCalls.o: nameserver/src/nameServCalls.cxx
	$(CCC) -c $(CCARCHFLAGS) -D$(ARCH) $<

CommProto.o: emacs_integration/src/CommProto.cpp
	$(CCC) -c $(CCARCHFLAGS) $(LOCAL_INCLUDES) $(BUILD_INCLUDES) -D$(ARCH) $<
