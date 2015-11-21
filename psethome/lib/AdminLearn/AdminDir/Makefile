.IGNORE:

OPTIONS=
PREFS=prefs/projectStructure.prefs
VERBOSELEVEL=5
EXEC=discover

start_discover: 
	start_discover -prefs $(PREFS) -Exec $(EXEC) -verboseLevel $(VERBOSELEVEL) $(OPTIONS)

sharedModel: 
	modelbuild -prefs $(PREFS) -Exec $(EXEC) -verboseLevel $(VERBOSELEVEL) $(OPTIONS)

prt: 
	modelbuild -prefs $(PREFS) -pdfExpand -Exec $(EXEC) $(OPTIONS) > junk.prt 2>&1

cleanPrivateModel:
	rm -rf model/$$USER

cleanSharedModel:
	find model -name '*.pset' -exec rm -f {} \;
	find model -name '*.pmod.*' -exec rm -f {} \;
	rm -rf model/SHARED/*/*
	rm -rf model/SHARED/*/*
	find src -name '*.pset' -exec rm -f {} \;
	find src -name '*.pmod.*' -exec rm -f {} \;

cleanFiles:
	rm -rf junk*
	rm -rf tmp/$$USER
	rm -f tmp/env_*
	rm -f tmp/log.*
	rm -f tmp/CPP*
	rm -f log/$$USER.*
	rm -f log/`date +%m%d%y`-*

cleanAll: cleanFiles cleanSharedModel cleanPrivateModel
	@echo "Done"
