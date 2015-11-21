VARIANT=-O

include $(MAKELIB)/Make.general

OBJECTS=admintool.o \
	SocketComm.o \
	nameServCalls.o

admintool: $(OBJECTS)
	$(CCC) -o $(@F) $(PRE_OBJ) $(CLDCTRL) -D$(ARCH) $(OBJECTS) $(CXXLIBS) $(SOCKETLIB) $(POST_OBJ)

admintool.o: admintool/src/admintool.cpp
	$(CCC) -c $(CCARCHFLAGS) -D$(ARCH) -I$(LOCAL_SRC_ROOT)/nameserver/include -I$(LOCAL_SRC_ROOT)/dish/include -I$(BUILD_SRC_ROOT)/nameserver/include -I$(BUILD_SRC_ROOT)/dish/include $<

SocketComm.o: dish/src/SocketComm.cxx
	$(CCC) -c $(CCARCHFLAGS) -D$(ARCH) $<

nameServCalls.o: nameserver/src/nameServCalls.cxx
	$(CCC) -c $(CCARCHFLAGS) -D$(ARCH) $<
