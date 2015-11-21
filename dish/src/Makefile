VARIANT=-O

include $(MAKELIB)/Make.general

OBJECTS=\
    dish.o \
    SocketComm.o \
    nameServCalls.o

dish2: $(OBJECTS)
	$(CCC) -o $@ $(CCARCHFLAGS) $(CLDCTRL) -D$(ARCH) $< SocketComm.o nameServCalls.o $(PRE_OBJ) $(SOCKETLIB) $(LDL) $(CXXLIBS) $(POST_OBJ)

dish.o: dish/src/dish.cxx
	$(CCC) -c $(CCARCHFLAGS) \
	    -I$(LOCAL_SRC_ROOT)/dish/include \
	    -I$(LOCAL_SRC_ROOT)/nameserver/include \
	    -I$(LOCAL_SRC_ROOT)/gen/include \
	    -I$(LOCAL_SRC_ROOT)/xxinterface/include \
	    -I$(BUILD_SRC_ROOT)/dish/include \
	    -I$(BUILD_SRC_ROOT)/nameserver/include \
	    -I$(BUILD_SRC_ROOT)/gen/include \
	    -I$(BUILD_SRC_ROOT)/xxinterface/include \
	    -D$(ARCH) $<

SocketComm.o: dish/src/SocketComm.cxx
	$(CCC) -c $(CCARCHFLAGS) -D$(ARCH) $<

nameServCalls.o: nameserver/src/nameServCalls.cxx
	$(CCC) -c $(CCARCHFLAGS) -D$(ARCH) $<
