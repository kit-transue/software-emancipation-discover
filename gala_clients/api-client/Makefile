all rfile afile rafile :
	@mkdir -p $(OBJDIR) $(OBJDIR)/obj
	cd $(OBJDIR) &&  $(MAKE) -f /paraset/src/clients/api-client/Makefile.`ParaArch` SRC=/paraset/src/clients/api-client  MAKEFLAGS=$(MAKEFLAGS) all

makemf makemfci depend xref xrefs clean includes :
	cd $(OBJDIR) && $(MAKE) -f /paraset/src/clients/api-client/Makefile.`ParaArch` SRC=/paraset/src/clients/api-client  $@
