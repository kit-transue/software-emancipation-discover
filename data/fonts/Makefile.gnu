MAXFILENAME = 20
ct=/usr/atria/bin/cleartool 

default:
	/paraset/admin/int_tools/bin/$(Arch)/gmake -f ./Makefile all

ifeq  "$(Arch)" "sun4"
ALL=aseticons.ff  Families.list aseticons20.fb aseticons20.snf fonts.dir
mkfontdir=/usr/bin/X11/mkfontdir
FONTDEP=aseticons20.snf
endif

ifeq  "$(Arch)" "sun5"
ALL=aseticons20.snf fonts.dir
FONTDEP=aseticons20.snf
mkfontdir=/usr/openwin/bin/mkfontdir
endif

ifeq  "$(Arch)" "hp700"
ALL=aseticons20.pcf  fonts.dir
FONTDEP=aseticons20.pcf
mkfontdir=/usr/bin/X11/mkfontdir
endif

ifeq "$(Arch)" "irix6"
ALL=aseticons20.pcf  fonts.dir
FONTDEP=aseticons20.pcf
mkfontdir=/usr/bin/X11/mkfontdir
endif

all:   $(ALL)
	- mv $(ALL) $(Arch)

aseticons.ff: aseticons20.fb Compat.list Synonyms.list
	@echo "### Making X11/NeWS font family"
	bldfamily -d . -f $(MAXFILENAME)

aseticons20.fb: aseticons20.bdf
	@echo "### Making X11/NeWS bitmap font"
	 convertfont -d . -f $(MAXFILENAME) -b -n aseticons aseticons20.bdf

fonts.dir: $(FONTDEP)
	#@echo "### Making X11 font directory"
	$(mkfontdir)

aseticons20.pcf : 
		/usr/bin/X11/bdftopcf aseticons20.bdf >aseticons20.pcf


aseticons20.snf: aseticons20.bdf
	@echo "### Making X11 Server Normal Format font"
	bdftosnf aseticons20.bdf > $@ 


