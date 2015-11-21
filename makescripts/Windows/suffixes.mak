.SUFFIXES:
.SUFFIXES: .obj .lib .cpp .cxx .c .i .mak

$(INTDIR)/%.obj:%.cpp
	$(CPP) $(CPLUSPLUS_FLAGS) $(CPP_PROJ) $<

$(INTDIR)/%.i:%.cpp
	$(CPP) /P $(CPLUSPLUS_FLAGS) $(CPP_PROJ) $<

$(INTDIR)/%.obj:%.cxx
	$(CPP) $(CPLUSPLUS_FLAGS) $(CPP_PROJ) $<

$(INTDIR)/%.i:%.cxx
	$(CPP) /P $(CPLUSPLUS_FLAGS) $(CPP_PROJ) $<

$(INTDIR)/%.obj:%.c
	$(CPP) $(CPP_PROJ) $<

$(INTDIR)/%.i:%.c
	$(CPP) /P $(CPP_PROJ) $<

$(INTDIR)/.%lib:%.mak
	$(MAKEEXE) $(MAKEOPT) -f $<

