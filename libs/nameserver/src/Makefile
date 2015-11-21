include $(MAKELIB)/Make.general

nameserv: nameServ.cxx
	$(CCC) -o $@ $(CCARCHFLAGS) -D$(ARCH) $< $(PRE_OBJ) $(CLDCTRL) $(SOCKETLIB) $(CXXLIBS) $(POST_OBJ)
