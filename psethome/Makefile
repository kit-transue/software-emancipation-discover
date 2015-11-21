include /paraset/psethome/Makelib/defs.mk

default: all

all: $(installdir) bin config doc emacs include java lib mb perl

$(installdir): FORCE
	mkdir -p $(installdir)

bin: FORCE
	$(PUTDIR) $(installdir) $@
	cd $(installdir)/$@ ; $(MAKE)

config: FORCE
	$(PUTDIR) $(installdir) $@
	cd $(installdir)/$@ ; $(MAKE)

doc: FORCE
	$(PUTDIR) $(installdir) $@
	cd $(installdir)/$@ ; $(MAKE)

emacs: FORCE
	$(PUTDIR) $(installdir) $@
	cd $(installdir)/$@ ; $(MAKE)

include: FORCE
	$(PUTDIR) $(installdir) $@
	cd $(installdir)/$@ ; $(MAKE)

java: FORCE
	$(PUTDIR) $(installdir) $@
	cd $(installdir)/$@ ; $(MAKE)

lib: FORCE
	$(PUTDIR) $(installdir) $@
	cd $(installdir)/$@ ; $(MAKE)

mb: FORCE
	$(PUTDIR) $(installdir) $@

perl: FORCE
	$(PUTDIR) $(installdir) $@
	cd $(installdir)/$@ ; $(MAKE)

FORCE:
