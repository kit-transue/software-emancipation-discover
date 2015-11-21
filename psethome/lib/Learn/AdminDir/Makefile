#
# Version /main/5
#
VERBOSELEVEL=0
EXEC=discover

view: 
	start_discover -prefs prefs/build.prefs -Exec $(EXEC) -verboseLevel $(VERBOSELEVEL)

build: 
	modelbuild -prefs prefs/build.prefs -Exec $(EXEC) -verboseLevel $(VERBOSELEVEL)

prt: 
	modelbuild -prefs prefs/build.prefs -pdfExpand -Exec $(EXEC) > junk.prt 2>&1

debugView:
	sh -xv `which start_discover` -prefs prefs/build.prefs -Exec $(EXEC) -verboseLevel $(VERBOSELEVEL) > junk.debug 2>&1

debugBuild:
	sh -xv `which modelbuild` -prefs prefs/build.prefs -Exec $(EXEC) -verboseLevel $(VERBOSELEVEL)> junk.debug 2>&1

debugPrt:
	sh -xv `which modelbuild` -prefs prefs/build.prefs -pdfExpand -Exec $(EXEC) > junk.debug 2>&1

cleanView:
	rm -rf $$ADMINDIR/model/private/*

cleanBuild:
	rm -rf $$ADMINDIR/model/SHARED/*

cleanFiles:
	rm -rf $$ADMINDIR/junk*
	rm -rf $$ADMINDIR/tmp/$$USER
	rm -f $$ADMINDIR/tmp/env_*
	rm -f $$ADMINDIR/tmp/log.*
	rm -f $$ADMINDIR/tmp/CPP*
	rm -f $$ADMINDIR/log/$$USER.*
	rm -f $$ADMINDIR/log/`date +%m%d%y`-*

