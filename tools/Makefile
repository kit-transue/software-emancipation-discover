# $Id: Makefile 1.2 1994/08/16 11:19:21EDT builder Exp  $

###########################################################################

SRCDIRS = \
	parser \
	config \
	cpp \
	paraset \
	g++filt \
        paracancel \
        paratotal \
	gcc \
        snifftool \
	frame \
	symtab


###########################################################################
# Utilities

CCC = /usr/local/CC/bin/CC
STAMP = $(TOOL)/stamp
LNDIRS = $(TOOL)/lndirs
REPORTCHANGED = if [ -f $@ ]; then echo 'Changed:'; ls $?; echo ''; fi
BUILDDATE = echo 'char build_date[] = "'"`date`"'";'
MACH = `ParaArch`

###########################################################################

# What to build if you just type "make"
default: all

# To rebuild all the executables
set: parser symtab aset aset_cpp g++filt paratotal paracancel config snifftool frame

aset aset.install aset.pure aset.quant aset.dir: FORCE
	make -f Makefile.aset.`ParaArch` $@

aset_cpp: FORCE
	make -f Makefile.aset_cpp.`ParaArch`

config: FORCE
	make -f Makefile.config.`ParaArch`

configurator configshell : FORCE
	make -f Makefile.config.`ParaArch` $@

g++filt: FORCE
	make -f Makefile.g++filt.`ParaArch`

frame: FORCE
	make -f Makefile.frame_api.`ParaArch`
	make -f Makefile.frame_srvr.`ParaArch`

snifftool: FORCE
	make -f Makefile.snifftool.`ParaArch`

paratotal: FORCE
	make -f Makefile.paratotal.`ParaArch`


paracancel: FORCE
	make -f Makefile.paracancel.`ParaArch`

parser: FORCE
	make -f Makefile.parser.`ParaArch`

symtab : FORCE
	make -f Makefile.symtab.`ParaArch` symtab


# Directories that may have to be created:
#$(LIB) $(INCL) $(ARCHBIN) $(ARCHLIB) $(ARCHDEV):
#	mkdir -p $@

# Use FORCE for targets that should be remade unconditionally.
FORCE:

# Recursive targets
# -----------------
# all: build object files in local directory
# depend: build dependency file (.depend)
# clean: remove all object files and temporary editor files
# makemf: build local Makefile based on existing source files
# afile: build archive files in $(LIB)
# rfile: build relocatable object files $(LIB)
# rafile: rfile and afile
# xref: update global cross-reference table
# includes: create links from local include to shared include directory

all depend clean clean.pset makemf afile rfile rafile includes: $(LIB) FORCE
	@ $(STAMP) STARTED   make $@ $(MAKELOG)
	@ for i in $(SRCDIRS); do ( \
	    if [ -d $$i ]; then \
		set -x; \
		cd $$i; $(MAKE) -f Makefile $@; \
	    fi; \
	) done
	@ $(STAMP) COMPLETED make $@ $(MAKELOG)

###########################################################################

# Warn users when their Makefile may be out of date.
Makefile: $(SYSBASE)/src/Makefile
	@ echo "WARNING: $(SYSBASE)/Makefile is newer than ./Makefile."

# Prepare environment for all executable targets
setup: Makefile $(ARCHBIN) $(ARCHDEV)

###########################################################################

