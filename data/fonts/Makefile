CV=`/usr/atria/bin/cleartool pwv -short`
all afile rfile default:
	if [ "$(Arch)" = hp700 ] ; then  \
		remsh fridge /usr/atria/bin/cleartool setview -exec \"cd /paraset/data/fonts \; make fonts\"  $(CV) ;\
		remsh tweety /usr/atria/bin/cleartool setview -exec \"cd /paraset/data/fonts \; make fonts \" $(CV) ;\
		remsh bigfoot /usr/atria/bin/cleartool setview -exec \"cd /paraset/data/fonts \; make fonts \" $(CV) ;\
	else \
		rsh fridge /usr/atria/bin/cleartool setview -exec \"cd /paraset/data/fonts \; make fonts\"  $(CV)  ;\
		rsh tweety /usr/atria/bin/cleartool setview -exec \"cd /paraset/data/fonts \; make fonts \" $(CV) ;\
		rsh bigfoot /usr/atria/bin/cleartool setview -exec \"cd /paraset/data/fonts \; make fonts \" $(CV) ;\
	fi

fonts:
	/paraset/admin/int_tools/bin/$(Arch)/gmake -f ./Makefile.gnu all
