OPTIONS=
VERBOSELEVEL=5
EXEC=discover

view: 
	start_discover -prefs prefs/build.prefs -Exec $(EXEC) -verboseLevel $(VERBOSELEVEL) $(OPTIONS)

build: 
	modelbuild -prefs prefs/build.prefs -Exec $(EXEC) -verboseLevel $(VERBOSELEVEL) $(OPTIONS)

prt: 
	modelbuild -prefs prefs/build.prefs -pdfExpand -Exec $(EXEC) $(OPTIONS) > junk.prt 2>&1

emulateView:
	emulate_discover -prefs prefs/build.prefs -Exec $(EXEC) -verboseLevel $(VERBOSELEVEL) $(OPTIONS)

emulateBuild:
	emulate_discover -prefs prefs/build.prefs -batch -Exec $(EXEC) -verboseLevel $(VERBOSELEVEL) $(OPTIONS)

debugEmulateView:
	sh -xv `emulate_discover -which` -prefs prefs/build.prefs -Exec $(EXEC) -verboseLevel $(VERBOSELEVEL) $(OPTIONS) > junk.debug 2>&1
debugEmulateBuild:
	sh -xv `emulate_discover -which` -prefs prefs/build.prefs -batch -Exec $(EXEC) -verboseLevel $(VERBOSELEVEL) $(OPTIONS) > junk.debug 2>&1

debugView:
	sh -xv `start_discover -which` -prefs prefs/build.prefs -Exec $(EXEC) -verboseLevel $(VERBOSELEVEL) $(OPTIONS) > junk.debug 2>&1

debugBuild:
	sh -xv `modelbuild -which` -prefs prefs/build.prefs -Exec $(EXEC) -verboseLevel $(VERBOSELEVEL) $(OPTIONS) > junk.debug 2>&1

debugPrt:
	sh -xv `modelbuild -which` -prefs prefs/build.prefs -pdfExpand -Exec $(EXEC) $(OPTIONS) > junk.debug 2>&1

cleanView:
	rm -rf model/$$USER/*

cleanBuild:
	rm -rf model/SHARED/*

cleanFiles:
	rm -rf junk*
	rm -rf tmp/$$USER
	rm -f tmp/env_*
	rm -f tmp/log.*
	rm -f tmp/CPP*
	rm -f log/$$USER.*
	rm -f log/`date +%m%d%y`-*

cleanAll: cleanFiles cleanBuild cleanView
	@echo "Done"
