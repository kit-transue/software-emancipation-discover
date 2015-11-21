# Makefile.install $Revision: 1.92 $
# 8.Sep.2000 Kit Transue
# attempt to describe dependencies between executables; describe top-level
# installations for DISCOVER/CodeRover, etc.

# must be fed to gmake: first section that sets variables uses gmake magic:

ADMMAKE=${PROJ_ROOT}/makescripts
VERSION=$(shell cat $(ADMMAKE)/version)
BINDIR=${DEST}/${Arch}-O/bin/${VERSION}
OBJDIR=${DEST}/${Arch}-O/obj
DOCS=DOCS
ADMINUI=ADMINUI
FLEX_ROOT=${PROJ_ROOT}/FLEXlm/flexlm/current

# NOTE: we do not support the special "sun55" symbol here, as we never
# intend to make an installer package for Solaris 2.5.1
ifeq "${Arch}" "sun5"
    LMGRARCH=sun4_u5
    LIBEXT=so
    SIARCH=sol2-sparc
endif

# If we are on HP-UX 10.20, no MKS libjava or Java 2
# So we can't run java apps, or expect to run the CI Admin GUI
# An ARCHV of "0" really means HP-UX 11.00
# TODO: make this more elegant
ifeq "${Arch}" "hp10"
    ifeq "${ARCHV}" "2"
    	LMGRARCH=hp700_u10
    	LIBEXT=sl
    	# We don't ship this for HP-UX 10
    	ADMINUI=
    	# No Java 2, so no jar to unzip the docs!
    	DOCS=
    else
    	LMGRARCH=hp700_u10
    	LIBEXT=sl
    	SIARCH=hpux11-parisc1
    endif
endif

ifeq "${Arch}" "irix6"
    LMGRARCH=sgi32_u6
    LIBEXT=so
    # Again, no IRIX 6.5 blimp libs.  No libjava for IRIX at all.
    SIARCH=UNKNOWN
    ADMINUI=
endif

# end gmake magic

# commands used to populate the installation:
INSTALL=$(ADMMAKE)/Unix/install/INSTALL
MKDIR=$(ADMMAKE)/Unix/install/MKDIR
LN=$(ADMMAKE)/Unix/install/LN
JAR=$(JAVA_HOME)/bin/jar

# touching makes this restartable, but there are often permissions problems
# for the partially-installed component.  Perhaps could be addressed in 
# INSTALL.
TOUCH_TARGET=echo Finished
#TOUCH_TARGET=touch


# Note: Discover is equivalent to DIScover
DISCOVER: PARSERS COMMUNICATIONS CONFIG EMACS DISCOVER_SH LEARN SIMPLIFY \
	GENERAL TPMCP MOTIF_UI DISLITE PARSERCMD DISMB QAR ASET_HELP TCL \
	AUTOFLAG FLEXLM ASET_DOCS PDFGEN DISIM CM_GENERIC PBMB \
	MESSAGES $(DOCS) LOGPRESENT XTOOLS $(ADMINUI)

	${INSTALL} ${BINDIR}/aset bin
	${INSTALL} ${BINDIR}/model_server bin
	${INSTALL} ${BINDIR}/paracancel bin
	# default .pdf if none specified:
	${INSTALL} ${PROJ_ROOT}/data/home.pdf lib
	${INSTALL} -RENAME ${PROJ_ROOT}/data/license.dat.template lib/license.dat
	# clean up all the .pjs
	find . -name '*.pj' -exec rm {} \;
	
	# Copy the installer ./setup file
	${INSTALL} ${PROJ_ROOT}/makescripts/Unix/install/setup .
	chmod a+x setup

	@${TOUCH_TARGET} $@

CODEROVER: PARSERS COMMUNICATIONS GENERAL MESSAGES TCL PARSERCMD DISMB \
	 FLEXLM XERCES PBMB EDITOR_INTERFACE MODEL_SERVER
	${INSTALL} ${OBJDIR}/DevXLauncher/DevXLauncher bin
	${INSTALL} ${BINDIR}/aset bin
	#${INSTALL} /view/doc_generation/DOCUMENTATION/external/enduser/development/common/coderover/eiguide/eiguide.pdf doc
	#${INSTALL} /view/doc_generation/DOCUMENTATION/external/enduser/development/common/coderover/ciguide/ciguide.pdf doc
	#${INSTALL} /view/doc_generation/DOCUMENTATION/external/enduser/development/common/coderover/refguide/reference.pdf doc
	#${INSTALL} ModelBuildGUI.jar lib
	#${INSTALL} CodeRover.jar lib
	#${INSTALL} CMDriver bin
	@${TOUCH_TARGET} $@

PBMB: PDFGEN
	${INSTALL} ${BINDIR}/scanclrmake bin
	${INSTALL} ${BINDIR}/mbdriver bin
	@${TOUCH_TARGET} $@

PDFGEN:
	${INSTALL} ${BINDIR}/pdfgen bin
	@${TOUCH_TARGET} $@

CALIPER:
	#${INSTALL} /view/doc_generation/DOCUMENTATION/external/enduser/development/common/caliper/caliper.pdf doc
	@${TOUCH_TARGET} $@

INTEGRITY:
	#${INSTALL} /view/doc_generation/DOCUMENTATION/external/enduser/development/common/integrity/integrity.pdf doc
	@${TOUCH_TARGET} $@

EDITOR_INTERFACE: EMACS_INTERFACE MSDEV_INTERFACE VIM_INTERFACE
	@${TOUCH_TARGET} $@

EMACS_INTERFACE:
	${INSTALL} ${OBJDIR}/EditorDrivers/editorserver bin
	${INSTALL} ${OBJDIR}/EditorDrivers/emacsclient bin
	# really put this in bin!?
	${INSTALL} ${PROJ_ROOT}/emacs_integration/src/roverserv.el bin
	@${TOUCH_TARGET} $@

MSDEV_INTERFACE:
	@${TOUCH_TARGET} $@

VIM_INTERFACE: VIM
	#${INSTALL} ${OBJDIR}/EditorDrivers/vimclient bin
	@${TOUCH_TARGET} $@

PSET_SERVER: GALA GENERAL
	${INSTALL} ${BINDIR}/pset_server bin
	@${TOUCH_TARGET} $@

MODEL_SERVER: GENERAL
	${INSTALL} ${BINDIR}/model_server bin
	@${TOUCH_TARGET} $@

PARSERCMD:
	${INSTALL} ${BINDIR}/parsercmd bin
	${INSTALL} ${PROJ_ROOT}/data/include/gcc.h include
	${INSTALL} ${PROJ_ROOT}/parsercmd/perl/parsercmds.pl mb
	${INSTALL} ${PROJ_ROOT}/parsercmd/perl/compilerid.pl mb
	${INSTALL} ${PROJ_ROOT}/parsercmd/perl/trycompiler.pl mb
	${INSTALL} ${PROJ_ROOT}/parsercmd/perl/trygcc.pl mb
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/acc.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/gcc.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/gccc++.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/gccc.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/gcccpp.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/hpansic.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/hpcc.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/hpcc11.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/javac.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/jbuilder.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/mipscc.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/mipsccpp.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/ntcl.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/ntclc.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/ntclcpp.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/ntclh.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/ntjvc.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/ntrc.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/object.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/passcc.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/passccpp.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/passinformixsql.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/passplsql.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/passsqlc.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/passsqlcpp.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/passsybasesql.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/suncc.xml lib
	${INSTALL} ${PROJ_ROOT}/parsercmd/data/sunccpp.xml lib
	@${TOUCH_TARGET} $@

LOGPRESENT:
	${INSTALL} ${PROJ_ROOT}/log_presentation/data/filtersev.xml lib
	@${TOUCH_TARGET} $@

DISCOVER_SH:
	# shell (usually) scripts that wrap the base executables:
	${INSTALL} ${PROJ_ROOT}/tools/discover bin -EXEC
	# used by/should probably incorporate into the discover script:
	${INSTALL} ${PROJ_ROOT}/tools/DISos bin -EXEC
	# used by discover script:
	${INSTALL} ${PROJ_ROOT}/tools/check_opts bin -EXEC
	${INSTALL} ${PROJ_ROOT}/tools/discover.reset bin -EXEC
	# more wrappers:
	${INSTALL} ${PROJ_ROOT}/psethome/bin/modelscript bin -EXEC
	${INSTALL} ${PROJ_ROOT}/psethome/bin/start_discover bin -EXEC
	${LN} bin/modelscript bin/modelbuild
	${LN} bin/modelscript bin/modeladmin
	${INSTALL} ${PROJ_ROOT}/psethome/bin/modelutil.sh bin -EXEC
	${INSTALL} ${PROJ_ROOT}/psethome/bin/util.sh bin -EXEC
	@${TOUCH_TARGET} $@

MESSAGES:
	${INSTALL} ${BINDIR}/log_presentation bin
	@${TOUCH_TARGET} $@

XSTUFF: APP_DEFAULTS XFONTS
	# $XKEYSYMDB is set to this by discover and read by aset:
	${INSTALL} ${PROJ_ROOT}/data/XKeysymDB	lib
	@${TOUCH_TARGET} $@

APP_DEFAULTS:
	${INSTALL} ${PROJ_ROOT}/data/app-defaults/Makefile	lib/app-defaults
	${INSTALL} ${PROJ_ROOT}/data/app-defaults/ParaSET.source	lib/app-defaults
	${INSTALL} ${PROJ_ROOT}/data/app-defaults/ParaSET-bw	lib/app-defaults
	${INSTALL} ${PROJ_ROOT}/data/app-defaults/ParaSET-color	lib/app-defaults
	${LN} lib/app-defaults/ParaSET-color	lib/app-defaults/DrawParaSET
	@${TOUCH_TARGET} $@

XFONTS:
	${MKDIR} lib/fonts
	(cd lib/fonts; ${PROJ_ROOT}/makescripts/Unix/install/install-fonts.sh)
	@${TOUCH_TARGET} $@

PARSERS:
	${INSTALL} ${BINDIR}/aset_CCcc bin
	${INSTALL} ${BINDIR}/ifext bin
	${INSTALL} ${BINDIR}/aset_jfe bin
	${INSTALL} ${BINDIR}/dis_sql bin
	${INSTALL} -RECURSE ${PROJ_ROOT}/data/include/aix include
	# options.map is called from smt_cppparm.C to translate gcc flags to EDG
	${INSTALL} ${PROJ_ROOT}/data/options.map lib
	${INSTALL} ${PROJ_ROOT}/data/valid_compiler_options_edg.dat lib
	@${TOUCH_TARGET} $@

GALA:
	${INSTALL} ${PROJ_ROOT}/model_server/graGala/src/symbols.new lib
	@${TOUCH_TARGET} $@

MOTIF_UI: XSTUFF
	# lists of things to display in the browser, with their queries.
	${INSTALL} ${PROJ_ROOT}/model_server/els/browser_data/* lib/browser_data
	${INSTALL} ${PROJ_ROOT}/model_server/cmd/src/assist_line.dat lib
	# thought this was part of gala, but called by graWorld::load_symbols
	${INSTALL} ${PROJ_ROOT}/model_server/gra/src/symbols.dat lib
	@${TOUCH_TARGET} $@

COMMUNICATIONS:
	${INSTALL} ${OBJDIR}/nameserv/nameserv bin
	${INSTALL} ${OBJDIR}/dish2/dish2 bin
	${INSTALL} ${OBJDIR}/serverspy/serverspy	bin
	${INSTALL} ${OBJDIR}/admintool/admintool	bin
	@${TOUCH_TARGET} $@

QAR:
	${INSTALL} ${BINDIR}/sevreporter bin
	${INSTALL} ${PROJ_ROOT}/sevreporter/scripts/* lib
	@${TOUCH_TARGET} $@

DISIM: DISMB
	# This is a temp work location -- the contents are distributed via
	# the disim.tar archive from now on
	${MKDIR} disim
	# Populate the working location
	(cd ${PROJ_ROOT}/DISIM; tar cf - bin lib template/ModelTemplate tmp/lockfiles/README.txt tmp/build_logs/README.txt model/README.txt) | (cd disim ; tar xf -)
	chmod u+xw disim/bin/StartDevx
	chmod u+xw disim/bin/StartBrowser
	chmod u+w  disim/tmp disim/model disim/template/ModelTemplate/*/* \
		disim/lib/Configuration.txt
	# move DISIM perlscripts and support files to modelbrowser dir
	cp disim/bin/admin/*.pl mb
	${INSTALL} disim/bin/admin/BrowserX.dis mb
	rm -rf disim/bin/
	rm -rf disim/bin/admin/fork
	${MKDIR} disim/bin/admin
	touch disim/bin/admin/ignore.me
	find disim -name '*.pj' -exec rm {} \;
	# create DISIM archive, and remove $PSETHOME/disim directory
	${MKDIR} share/DISIM
	(cd disim; ${JAR} cMf ../share/DISIM/DISIM.zip bin lib model template tmp)
	rm -rf disim
	${INSTALL} ${PROJ_ROOT}/psethome/bin/BuildModel bin -EXEC
	${INSTALL} ${PROJ_ROOT}/psethome/bin/CreateSummary bin -EXEC
	${INSTALL} ${PROJ_ROOT}/psethome/bin/NTPSETServerAdmin bin -EXEC
	${INSTALL} ${PROJ_ROOT}/psethome/bin/QueryResumability bin -EXEC
	${INSTALL} ${PROJ_ROOT}/psethome/bin/RemoveModel bin -EXEC
	${INSTALL} ${PROJ_ROOT}/psethome/bin/RunQAR bin -EXEC
	${INSTALL} ${PROJ_ROOT}/psethome/bin/RunSimplify bin -EXEC
	${INSTALL} ${PROJ_ROOT}/psethome/bin/ServerAdmin bin -EXEC
	${INSTALL} ${PROJ_ROOT}/psethome/bin/VersionWindow bin -EXEC
	${INSTALL} ${PROJ_ROOT}/DISIM/bin/StartBuildServer bin -EXEC
	${INSTALL} ${PROJ_ROOT}/DISIM/bin/StartBrowser bin -EXEC
	${INSTALL} ${PROJ_ROOT}/DISIM/bin/StartDevx bin -EXEC
	${INSTALL} ${PROJ_ROOT}/DISIM/lib/scripts/BatchSimplify.dis lib
	@${TOUCH_TARGET} $@

GENERAL:
	# I'm not sure where this goes!
	${INSTALL} ${BINDIR}/ci_environment bin
	# Supposition is that these are used for pset diffs:
	${INSTALL} ${BINDIR}/mrg_diff bin
	${INSTALL} ${BINDIR}/mrg_update bin
	# flex is used for anything that reads a PDF:
	${INSTALL} ${OBJDIR}/flex/flex bin
	${INSTALL} ${PROJ_ROOT}/flex/src/flex.skel lib
	# The copyright page refers to the bug reporting template:
	${INSTALL} ${PROJ_ROOT}/data/BugTemplate lib
	# pset-which appears used by discover.reset, dislite:
	${INSTALL} ${PROJ_ROOT}/makescripts/bin/${Arch}/pset-which bin
	# checklist is an optional PDF selector:
	${INSTALL} ${BINDIR}/checklist bin
	# realname is used in Customize class in aset/et.al. preferences:
	${INSTALL} ${PROJ_ROOT}/tools/realname bin -EXEC
	# DISarch used by discover, slay, Learn, ...?
	${INSTALL} ${PROJ_ROOT}/tools/DISarch bin -EXEC
	# slay probably hasn't been updated in a while, but put it in?
	${INSTALL} -RENAME ${PROJ_ROOT}/tools/slay.sh bin/slay -EXEC
	# ParaDOCS now used solely for report-templates, but loaded by all PDFs:
	${INSTALL} ${PROJ_ROOT}/data/ParaDOCS.pdf lib
	# messages.dat only required until message overhaul complete:
	${INSTALL} ${PROJ_ROOT}/data/messages.dat lib
	# msg_database.dat for the Motif UI (so far):
	${INSTALL} ${PROJ_ROOT}/data/msg_database.dat lib
	# default preferences are required:
	${INSTALL} -RENAME ${PROJ_ROOT}/data/settings/psetPrefs/psetPrefs.unix lib/psetPrefs.default
	# General support for Access:
	${INSTALL} ${PROJ_ROOT}/model_server/api/scripts/*.dis lib
	${INSTALL} ${PROJ_ROOT}/model_server/api/scripts/exclcmds.txt lib
	@${TOUCH_TARGET} $@

TPMCP:
	${INSTALL} ${BINDIR}/mrg_ui bin
	${INSTALL} ${PROJ_ROOT}/mrg_ui/scripts/mrg_ui.tcl lib
	@${TOUCH_TARGET} $@

ASET_HELP: HYPERHELP
#	/usr/atria/bin/cleartool startview doc_generation
#	${INSTALL} /DOCUMENTATION/external/enduser/development/unix/05help/*.cnt lib/hyperhelp
	@${TOUCH_TARGET} $@

ASET_DOCS:
	# these refcards are getting pretty dated!
	#${INSTALL} ${PROJ_ROOT}/contrib/gnu_emacs_refcard.ps.Z lib
	# It's time to list the docs explicitly:
	#${INSTALL} /view/doc_generation/DOCUMENTATION/external/enduser/development/common/access/access.pdf doc
	#${INSTALL} /view/doc_generation/DOCUMENTATION/external/enduser/development/unix/03adminguide/aguide.pdf doc
	#${INSTALL} /view/doc_generation/DOCUMENTATION/external/enduser/development/unix/04adminref/aref.pdf doc
	#${INSTALL} /view/doc_generation/DOCUMENTATION/external/enduser/development/common/parser/parser.pdf doc
	#${INSTALL} /view/doc_generation/DOCUMENTATION/external/enduser/development/common/parserref/parserref.pdf doc
	#${INSTALL} /view/doc_generation/DOCUMENTATION/external/enduser/development/unix/08quickref/quickref.pdf doc
	#${INSTALL} /view/doc_generation/DOCUMENTATION/external/enduser/development/common/relnotes/relnotes.pdf doc
	#${INSTALL} /view/doc_generation/DOCUMENTATION/external/enduser/development/unix/01userguide/uguide.pdf doc
	@${TOUCH_TARGET} $@


SIMPLIFY:
	${INSTALL} ${BINDIR}/simplify bin -EXEC
	${INSTALL} ${PROJ_ROOT}/thirdparty/jre/${Arch}/dis_jre bin -EXEC
	${INSTALL} ${PROJ_ROOT}/extlib/jars/discover.jar lib
	${INSTALL} ${PROJ_ROOT}/thirdparty/jre/${Arch}/swingall.jar lib
	@${TOUCH_TARGET} $@

LEARN: RCS
	${INSTALL} ${PROJ_ROOT}/psethome/bin/InstallAdminLearn bin -EXEC
	${INSTALL} -RECURSE ${PROJ_ROOT}/psethome/lib/AdminDir lib
	${INSTALL} -RECURSE ${PROJ_ROOT}/psethome/lib/AdminLearn lib
	${INSTALL} -RECURSE ${PROJ_ROOT}/psethome/lib/Learn lib
	find lib -name '*.pset' -print | xargs chmod u+w
	find lib -name '*.pset' -print | xargs touch
	@${TOUCH_TARGET} $@

DISLITE: VIM
	# I think egrep is used only for dislite:
	${INSTALL} ${PROJ_ROOT}/makescripts/bin/${Arch}/egrep bin/generic
	${INSTALL} ${PROJ_ROOT}/dislite/scripts/dislite bin -EXEC
	${INSTALL} ${PROJ_ROOT}/dislite/scripts/disviper bin -EXEC
	${INSTALL} ${PROJ_ROOT}/dislite/scripts/*.tcl lib/dislite
	chmod u+x lib/dislite/appClient.tcl lib/dislite/appServer.tcl
	${INSTALL} ${PROJ_ROOT}/dislite/scripts/*.el lib/dislite
	${INSTALL} ${PROJ_ROOT}/dislite/scripts/*.dat lib/dislite
	${INSTALL} ${PROJ_ROOT}/dislite/scripts/*.xbm lib/dislite
	${INSTALL} ${PROJ_ROOT}/dislite/scripts/disvim bin -EXEC
	#${INSTALL} /view/doc_generation/DOCUMENTATION/external/enduser/development/common/devXpress/devx.pdf doc
	${INSTALL} ${OBJDIR}/emacsAdapter/emacsAdapter bin/
	${INSTALL} ${OBJDIR}/devxAdapter/devxAdapter bin/
	${INSTALL} ${PROJ_ROOT}/emacs_integration/src/emacsIntegration.el bin
	@${TOUCH_TARGET} $@

CONFIG: CONFIG_RCS CONFIG_COPY_CM CONFIG_SCCS CONFIG_CLEARCASE CONFIG_CONTINUUS CONFIG_CVS
	${MKDIR} config/custom
	${INSTALL} ${OBJDIR}/configshell/configshell    bin
	${INSTALL} ${OBJDIR}/configurator/configurator  bin
	# setwait is called from configshell, and is a nasty ugly hack:
	${INSTALL} ${PROJ_ROOT}/tools/setwait bin -EXEC
	@${TOUCH_TARGET} $@

CONFIG_RCS:
	${INSTALL} ${PROJ_ROOT}/config/config/rcs_src/* config/rcs -EXEC
	${INSTALL} ${PROJ_ROOT}/config/config/rcs_src/util/* config/rcs/util -EXEC
	${INSTALL} ${OBJDIR}/config/rcs_src/* config/rcs/util -EXEC
	@${TOUCH_TARGET} $@

CONFIG_COPY_CM:
	${INSTALL} ${PROJ_ROOT}/config/config/copy_cm/* config/copy_cm -EXEC
	@${TOUCH_TARGET} $@

CONFIG_SCCS:
	# we used to treat HP differently, but I see no reason to:
	${INSTALL} ${PROJ_ROOT}/config/config/sccs/* config/sccs -EXEC
	${INSTALL} ${PROJ_ROOT}/config/config/sccs/util/* config/sccs/util -EXEC
	${INSTALL} ${OBJDIR}/config/sccs/* config/sccs/util -EXEC
	@${TOUCH_TARGET} $@

CONFIG_CLEARCASE:
	${INSTALL} ${PROJ_ROOT}/config/config/clearcase/* config/clearcase -EXEC
	${INSTALL} ${PROJ_ROOT}/config/config/clearcase/util/* config/clearcase/util -EXEC
	${INSTALL} ${OBJDIR}/config/clearcase/* config/clearcase/util -EXEC
	@${TOUCH_TARGET} $@

CONFIG_CONTINUUS:
	${INSTALL} ${PROJ_ROOT}/config/config/continuus/* config/continuus -EXEC
	${INSTALL} ${PROJ_ROOT}/config/config/continuus/util/* config/continuus/util -EXEC
	${INSTALL} ${OBJDIR}/config/continuus/* config/continuus/util -EXEC
	@${TOUCH_TARGET} $@

CONFIG_CVS:
	${INSTALL} ${PROJ_ROOT}/config/config/cvs/* config/cvs -EXEC
	${INSTALL} ${PROJ_ROOT}/config/config/cvs/util/* config/cvs/util -EXEC
	${INSTALL} ${OBJDIR}/config/cvs/* config/cvs/util -EXEC
	@${TOUCH_TARGET} $@

DISMB: PERL
	${INSTALL} ${BINDIR}/dismb bin
	${INSTALL} ${PROJ_ROOT}/psethome/mb/* mb
	${INSTALL} ${BINDIR}/disperl bin
	# dis_path was introduced with Perl; seems to be related:
	${INSTALL} ${BINDIR}/dis_path bin
	# used at least with dismb:
	${INSTALL} ${OBJDIR}/lock/dis_lock bin
	${INSTALL} ${OBJDIR}/md5/md5 bin
	# appears to be DISMB related:
	${INSTALL} ${PROJ_ROOT}/psethome/bin/disadmin	bin -EXEC
	# whacky script referenced only by dismb:
	${INSTALL} ${PROJ_ROOT}/psethome/bin/disshell	bin -EXEC
	@${TOUCH_TARGET} $@

CM_GENERIC: XERCES
	${INSTALL} ${BINDIR}/cmdriver bin
	${INSTALL} ${BINDIR}/cmdish bin
	${INSTALL} ${PROJ_ROOT}/cmdriver/CmXml/Unix/cm.xml lib
	${INSTALL} ${PROJ_ROOT}/cmdriver/CmXml/Unix/si_get_sandboxes bin -EXEC
	${INSTALL} ${PROJ_ROOT}/cmdriver/CmXml/Unix/rcs_ci bin -EXEC
	${INSTALL} ${PROJ_ROOT}/cmdriver/CmXml/Unix/rcs_co bin -EXEC
	${INSTALL} ${PROJ_ROOT}/cmdriver/CmXml/Unix/rcs_unco bin -EXEC

#############################
# major external libraries
#############################

PERL:
	${MKDIR} perl
	(cd perl; tar xf ${PROJ_ROOT}/thirdparty/perl/${ARCH}/perl.tar)
	${INSTALL} ${BINDIR}/disperl	bin
	@${TOUCH_TARGET} $@

JAVA:
	# TODO: Determine if this target is used anymore
	${MKDIR} java
	(cd java; tar xf ${PROJ_ROOT}/thirdparty/jre/${ARCH}/jre.tar)
	@${TOUCH_TARGET} $@

JAVA2:
	${MKDIR} jre
	# Grab jre from $$JAVA_HOME
	(cd ${JAVA_HOME}/jre; tar cf - .) | (cd jre; tar xf -)
	@${TOUCH_TARGET} $@

EMACS:
	${MKDIR} emacs
	(cd emacs; tar xf ${PROJ_ROOT}/thirdparty/emacs/${ARCH}/emacs.tar)
	@${TOUCH_TARGET} $@

TCL:
	${INSTALL} -RECURSE ${PROJ_ROOT}/dis_tk8.3/library/* lib/tk8.3
	${INSTALL} -RECURSE ${PROJ_ROOT}/dis_tcl8.3/library/* lib/tcl8.3
	${INSTALL} ${OBJDIR}/dis_tk/wish bin
	${INSTALL} ${OBJDIR}/dis_tcl/tclsh bin
	@${TOUCH_TARGET} $@

AUTOFLAG:
	${INSTALL} -RECURSE ${PROJ_ROOT}/psethome/lib/autoflag/* lib/autoflag
	@${TOUCH_TARGET} $@

VIM:
	${INSTALL} ${OBJDIR}/vim/vim bin -EXEC
	${INSTALL} ${PROJ_ROOT}/vim-5.3/README.txt lib/vim
	${INSTALL} ${PROJ_ROOT}/vim-5.3/bugreport.vim lib/vim
	${INSTALL} -RECURSE ${PROJ_ROOT}/vim-5.3/doc lib/vim
	${INSTALL} ${PROJ_ROOT}/vim-5.3/gvimrc_example lib/vim
	${INSTALL} ${PROJ_ROOT}/vim-5.3/menu.vim lib/vim
	${INSTALL} ${PROJ_ROOT}/vim-5.3/discover.vim lib/vim
	${INSTALL} -RECURSE ${PROJ_ROOT}/vim-5.3/syntax lib/vim
	${INSTALL} -RECURSE ${PROJ_ROOT}/vim-5.3/tools lib/vim
	chmod u+x lib/vim/tools/vim132 lib/vim/tools/vimm lib/vim/tools/vimspell.sh
	${INSTALL} -RECURSE ${PROJ_ROOT}/vim-5.3/tutor lib/vim
	${INSTALL} ${PROJ_ROOT}/vim-5.3/vimrc_example lib/vim
	@${TOUCH_TARGET} $@

HYPERHELP:
	# make the real executable live in lib/hyperhelp:
	#${INSTALL} ${HHHOME}/bin/hyperhelp lib/hyperhelp/bin
	#${LN} lib/hyperhelp/bin/hyperhelp bin/hyperhelp
	#${INSTALL} ${HHHOME}/hoh.hlp lib/hyperhelp
	#${INSTALL} -RECURSE /net/oxen/vol03/share/sun5/bristol/hyperhelp/xprinter lib/hyperhelp
	#${INSTALL} -RECURSE ${HHHOME}/app-defaults lib/hyperhelp
	#${INSTALL} -RECURSE ${HHHOME}/lib lib/hyperhelp
	#@${TOUCH_TARGET} $@

FLEXLM:
	${INSTALL} ${FLEX_ROOT}/${LMGRARCH}/lmgrd       bin -EXEC
	${INSTALL} ${FLEX_ROOT}/${LMGRARCH}/MKS         bin -EXEC
	${INSTALL} ${FLEX_ROOT}/${LMGRARCH}/lmutil      bin -EXEC
	${LN} bin/lmutil                        bin/lmcksum
	${LN} bin/lmutil                        bin/lmdiag
	${LN} bin/lmutil                        bin/lmdown
	${LN} bin/lmutil                        bin/lmhostid
	${LN} bin/lmutil                        bin/lminstall
	${LN} bin/lmutil                        bin/lmremove
	${LN} bin/lmutil                        bin/lmreread
	${LN} bin/lmutil                        bin/lmstat
	${LN} bin/lmutil                        bin/lmswitchr
	${LN} bin/lmutil                        bin/lmver
	@${TOUCH_TARGET} $@

RCS:
	${INSTALL} ${PROJ_ROOT}/thirdparty/rcs_cm_src/${Arch}/src/ci        bin -EXEC
	${INSTALL} ${PROJ_ROOT}/thirdparty/rcs_cm_src/${Arch}/src/co        bin -EXEC
	${INSTALL} ${PROJ_ROOT}/thirdparty/rcs_cm_src/${Arch}/src/ident     bin -EXEC
	${INSTALL} ${PROJ_ROOT}/thirdparty/rcs_cm_src/${Arch}/src/merge     bin -EXEC
	${INSTALL} ${PROJ_ROOT}/thirdparty/rcs_cm_src/${Arch}/src/rcs.exe   bin/rcs
	${INSTALL} ${PROJ_ROOT}/thirdparty/rcs_cm_src/${Arch}/src/rcsdiff   bin -EXEC
	${INSTALL} ${PROJ_ROOT}/thirdparty/rcs_cm_src/${Arch}/src/rcsmerge  bin -EXEC
	${INSTALL} ${PROJ_ROOT}/thirdparty/rcs_cm_src/${Arch}/src/rlog      bin -EXEC
	${INSTALL} ${OBJDIR}/diff/cmp                                       bin
	${INSTALL} ${OBJDIR}/diff/sdiff                                     bin
	${INSTALL} ${OBJDIR}/diff/diff                                      bin
	${INSTALL} ${OBJDIR}/diff/diff3                                     bin
	@${TOUCH_TARGET} $@

XERCES:
	${INSTALL} ${PROJ_ROOT}/extlib/${Arch}/libxerces-c1_2.${LIBEXT} lib -EXEC
	${INSTALL} ${PROJ_ROOT}/extlib/${Arch}/libicu-uc.${LIBEXT} lib -EXEC
	${INSTALL} ${PROJ_ROOT}/extlib/${Arch}/libicudata.${LIBEXT} lib -EXEC
	@${TOUCH_TARGET} $@





######################################################
# Stuff that can be (has been?) definitively removed:
######################################################

OBSOLETE: XVFB AUTODOC DEFECTLINK EXTRACT PACKAGE TESTLINK FRAME_INTEGRATION STATISTICS Y2K
	# misc.sh seems to wrap a function with debugging information; unreferences?
	bin/misc.sh
	# Reggie's diff-betweeen-installations:
	bin/apply_patch
	# Daryl's pdfgen perl script:
	bin/pdfgen
	# won't worry about pure stuff:
	bin/aset.pure
	# atria_filter is installed under config/clearcase; clearcase cm scripts reference it there:
	bin/atria_filter
	# we have better ways of counting LOC than cexec:
	bin/cexec
	# Autotrial flags database.  We can do better with flags translator:
	bin/compilers.dat
	# Remove LOC licensing:
	bin/discrypt
	# These are best replaced with Access functionality:
	bin/pset_file_status
	bin/pset_loc
	bin/pset_proj_status
	bin/pset_status
	# and checkpmod option has been thankfully removed:
	bin/ptest_checkpmod
	# removed gcc-based include files:
	${MKDIR} include/cp-include
	include/cp-include/stdarg.h
	include/stdarg.h
	include/va-sparc.h
	include/varargs.h
	# and the gcc valid compiler options:
	# currently used--see bug19862
	# lib/valid_compiler_options.dat
	# stub directories from mixed mode stuff:
	${MKDIR} lib/cfront
	${MKDIR} lib/gcc
	${MKDIR} lib/sparcworks
	# boilerplates obsolete with forward engineering:
	${MKDIR} lib/sample-boilerplates
	lib/sample-boilerplates/before-class-decl_paraset_sample
	lib/sample-boilerplates/before-class-def_paraset_sample
	lib/sample-boilerplates/before-func-def_paraset_sample
	lib/sample-boilerplates/before-member-def_paraset_sample
	lib/sample-boilerplates/before-struct_paraset_sample
	# icons directory contains only source icons--not used?
	${MKDIR} lib/icons
	lib/icons/arrow13.pxm
	lib/icons/dis_stop13.pxm
	lib/icons/stop13.pxm
	# gdb no longer shipped with installation:
	lib/gdb_refcard.ps.Z
	# csh script Learn.setup moved into discover bourne shell:
	${INSTALL} ${PROJ_ROOT}/tools/Learn.setup bin
	# Learn.install was so simple it made sense to move into discover(sh):
	${INSTALL} ${PROJ_ROOT}/tools/Learn.install bin
	# bgpid was used by modelscript to background vfb:
	${INSTALL} ${PROJ_ROOT}/psethome/bin/bgpid.sh bin
	# gcc-based valid_compiler_options.dat is no longer referenced:
	${INSTALL} ${PROJ_ROOT}/data/valid_compiler_options.dat lib
	@${TOUCH_TARGET} $@

XVFB:
	${INSTALL} $(OBJDIR)/xutil/xopen bin
	${INSTALL} $(OBJDIR)/xutil/xscan bin
	${INSTALL} ${PROJ_ROOT}/bin/${Arch}/disXvfb bin
	@${TOUCH_TARGET} $@

AUTODOC:
	${INSTALL} -RECURSE ${PROJ_ROOT}/data/report-templates lib
	${INSTALL} ${PROJ_ROOT}/build/software/copy_reportplates bin
	@${TOUCH_TARGET} $@

DEFECTLINK:
	${INSTALL} ${PROJ_ROOT}/src/DefectProducts/DefectLink/*.pl bin
	${INSTALL} ${PROJ_ROOT}/src/DefectProducts/DefectLink/*.dis lib
	${INSTALL} ${PROJ_ROOT}/src/DefectProducts/ProblemTracking/dis_PT_main.tcl lib
	${INSTALL} ${PROJ_ROOT}/src/DefectProducts/ProblemTracking/dis_PT_get_regions.pl bin
	@${TOUCH_TARGET} $@

EXTRACT:
	${INSTALL} ${PROJ_ROOT}/data/settings/subsysPrefs/subsysPrefs.default lib
	@${TOUCH_TARGET} $@

PACKAGE:
	${INSTALL} ${BINDIR}/package bin/package
	${INSTALL} ${BINDIR}/pksym.index bin/package
	${INSTALL} ${BINDIR}/pksym.index.rtl bin/package
	${INSTALL} ${BINDIR}/qvparser bin/package
	${INSTALL} ${BINDIR}/gxpackage bin/package
	${INSTALL} ${BINDIR}/pcparser bin/package
	${INSTALL} ${BINDIR}/vcparser bin/package
	${INSTALL} ${BINDIR}/helper bin/package
	${INSTALL} ${PROJ_ROOT}/src/package/batch/psr/*.psr	bin/package/psr
	${INSTALL} ${PROJ_ROOT}/src/package/batch/scripts/*	bin/package
	${INSTALL} ${PROJ_ROOT}/src/package/gui/X/xDefaults	bin/package
	#${INSTALL} "/view/doc_generation/DOCUMENTATION/external/enduser/development/unix/05help/PACKAGE.hlp" lib
	#${INSTALL} "/view/doc_generation/DOCUMENTATION/external/enduser/development/unix/05help/PACKAGE.fts" lib
	@${TOUCH_TARGET} $@

TESTLINK:
	${INSTALL} ${BINDIR}/testlink bin/tests
	@${TOUCH_TARGET} $@

FRAME_INTEGRATION:
	${INSTALL} ${OBJDIR}/frame_api/frame_api bin
	${INSTALL} ${OBJDIR}/frame_srvr/frame_srvr bin
	@${TOUCH_TARGET} $@

STATISTICS:
	${INSTALL} ${PROJ_ROOT}/model_server/interface/src/discover.summary.dis lib
	@${TOUCH_TARGET} $@

Y2K:
	${INSTALL} ${PROJ_ROOT}/data/default.ptn lib
	${INSTALL} ${PROJ_ROOT}/data/default.flt lib
	@${TOUCH_TARGET} $@

DOCS:
	# Copy Access, Model Browser and Admin UI webhelps into install
	# TODO: Sort out HP-UX 10 vs. 11 issues.  10.20 will dump core
	# when running any Java 2 command.  Run this only on HP-UX
	# 11.00B for now
	${MKDIR} webhelp/access
	${MKDIR} webhelp/model_browser
	${MKDIR} webhelp/admin_client
	(cd webhelp/access; ${JAR} xf ${PROJ_ROOT}/Docs/webhelp/unix_access_webhelp.zip)
	(cd webhelp/model_browser; ${JAR} xf ${PROJ_ROOT}/Docs/webhelp/unix_model_browser_webhelp.zip)
	(cd webhelp/admin_client; ${JAR} xf ${PROJ_ROOT}/Docs/webhelp/admin_client_webhelp.zip)

	@${TOUCH_TARGET} $@

ADMINUI: JAVA2
	# Copy the SI Blimp objects into the install location
	# TODO: Figure out what to do about IRIX, HP-UX 10.20
	${INSTALL} ${BINDIR}/CIEClient bin
	${INSTALL} ${SOLUTION_BUILD_ROOT}/solution/components/libjava/dest/native/${SIARCH}/libmksIO.${LIBEXT} bin
	${INSTALL} ${SOLUTION_BUILD_ROOT}/solution/components/libjava/dest/native/${SIARCH}/libmksUtil.${LIBEXT} bin
	${INSTALL} ${SOLUTION_BUILD_ROOT}/solution/components/framework/dest/native/${SIARCH}/cie bin

	# Copy the CI Admin classes and resources 
	${INSTALL} ${PROJ_ROOT}/adminui/dest/lib/CIAdmin.jar lib
	${INSTALL} ${PROJ_ROOT}/adminui/dest/lib/wizard.xml lib
	${INSTALL} ${PROJ_ROOT}/adminui/dest/lib/setuptxt.xsl lib
	${INSTALL} -RECURSE ${PROJ_ROOT}/adminui/dest/resources/* resources

	@${TOUCH_TARGET} $@

XTOOLS:
	${INSTALL} ${PROJ_ROOT}/tools/ptest/virtual_display_tools/${Arch}/xopen bin -EXEC
	@${TOUCH_TARGET} $@

