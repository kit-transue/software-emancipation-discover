VARIANT=-O

include $(MAKELIB)/Make.general

INCLUDES=\
    -I$(LOCAL_SRC_ROOT)/nameserver/include\
    -I$(LOCAL_SRC_ROOT)/dish/include\
    -I$(LOCAL_SRC_ROOT)/stream_message/include\
    -I$(BUILD_SRC_ROOT)/nameserver/include\
    -I$(BUILD_SRC_ROOT)/dish/include\
    -I$(BUILD_SRC_ROOT)/stream_message/include

OBJECTS	= serverspy.o \
	  nameServCalls.o \
          SocketComm.o \
          startprocess.o

serverspy: $(OBJECTS)
	$(CCC) -o $(@F) $(PRE_OBJ) $(CLDCTRL) $(OBJECTS) $(CXXLIBS) $(SOCKETLIB) $(POST_OBJ)

serverspy.o: serverspy/src/serverspy.cxx
	$(CCC) -c $(CCARCHFLAGS) $(INCLUDES) -D$(ARCH) -D__EXTENSIONS__ $<

nameServCalls.o: nameserver/src/nameServCalls.cxx
	$(CCC) -c $(CCARCHFLAGS) -D$(ARCH) -D__EXTENSIONS__ $<

SocketComm.o: dish/src/SocketComm.cxx
	$(CCC) -c $(CCARCHFLAGS) -D$(ARCH) -D__EXTENSIONS__ $<

startprocess.o: stream_message/src/startprocess.cxx
	$(CCC) -c $(CCARCHFLAGS) $(INCLUDES) -D$(ARCH) -D__EXTENSIONS__ -DDIRECT_MSG $<
