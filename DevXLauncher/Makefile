
include $(MAKELIB)/Make.general

NAMESERV_INC = -I$(LOCAL_SRC_ROOT)/nameserver/include -I$(BUILD_SRC_ROOT)/nameserver/include

DEVX_INC = -I$(LOCAL_SRC_ROOT)/DevXLauncher/include -I$(BUILD_SRC_ROOT)/DevXLauncher/include $<

OBJECTS	= DevXLauncher.o \
	StartExe.o \
	nameServCalls.o \
	startproc.o

DevXLauncher: $(OBJECTS)
	$(CCC) -o $(@F) $(PRE_OBJ) $(CLDCTRL) -D$(ARCH) $(OBJECTS) $(CXXLIBS) $(SOCKETLIB) $(POST_OBJ)

DevXLauncher.o: DevXLauncher/src/DevXLauncher.cpp
	$(CCC) -c $(CCARCHFLAGS) -D$(ARCH) $(DEVX_INC) $<

StartExe.o: StartExe/StartExe.cpp
	$(CCC) -c $(CCARCHFLAGS) -D$(ARCH) $(NAMESERV_INC) $(DEVX_INC) $<

nameServCalls.o: nameserver/src/nameServCalls.cxx
	$(CCC) -c $(CCARCHFLAGS) -D$(ARCH) $<

startproc.o: DevXLauncher/src/startproc.cpp
	$(CCC) -c $(CCARCHFLAGS) -D$(ARCH) $(DEVX_INC) $<
