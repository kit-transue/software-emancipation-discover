include /paraset/psethome/Makelib/defs.mk

default: all

FORCE:

all: tcl8.0 tk8.0 gala_scripts browser_data dislite data_files

data_files: home.pdf ParaDOCS.pdf messages.dat options.map valid_compiler_options.dat valid_compiler_options_edg.dat default.ptn settings_files XKeysymDB license.dat

settings_files: psetPrefs.default subsysPrefs.default

tcl8.0: FORCE
	mkdir -p tcl8.0
	tar cf - /paraset/src/tcl8.0/library/* | (cd tcl8.0 ; tar xvf -)

tk8.0: FORCE
	mkdir -p tk8.0
	tar cf - /paraset/src/tk8.0/library/* | (cd tk8.0 ; tar xvf -)

home.pdf: /paraset/data/home.pdf
	cp /paraset/data/home.pdf .

ParaDOCS.pdf: /paraset/data/ParaDOCS.pdf
	cp /paraset/data/ParaDOCS.pdf .

messages.dat: /paraset/data/messages.dat
	cp /paraset/data/messages.dat .

options.map: /paraset/data/options.map
	cp /paraset/data/options.map .

valid_compiler_options.dat: /paraset/data/valid_compiler_options.dat
	cp /paraset/data/valid_compiler_options.dat .

valid_compiler_options_edg.dat: /paraset/data/valid_compiler_options_edg.dat
	cp /paraset/data/valid_compiler_options_edg.dat .

default.ptn: /paraset/data/default.ptn
	cp /paraset/data/default.ptn .

psetPrefs.default: /paraset/data/psetPrefs/psetPrefs.unix
	cp /paraset/data/psetPrefs/psetPrefs.unix ./psetPrefs.default

subsysPrefs.default: /paraset/data/psetPrefs/subsysPrefs.default
	cp /paraset/data/psetPrefs/subsysPrefs.default .

XKeysymDB: /paraset/data/XKeysymDB
	cp /paraset/data/XKeysymDB .

license.dat: /paraset/data/license.dat.template
	cp /paraset/data/license.dat.template .

gala_scripts: FORCE
	mkdir -p gala_scripts
	tar cf -

browser_data: FORCE
	mkdir -p browser_data
	tar cf - /paraset/src/paraset/els/browser_data/* | (cd browser_data ; tar xvf -)
