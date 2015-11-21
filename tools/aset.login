#######################################################################
# Variables for developing ParaSET
#
# NOTE :: this file is maintained through the vob /paraset .
#         It is copied to /ParaSET as a convenience for times when
#         access outside of a clearcase view is needed.
#
#         DO NOT MAKE CHANGES to /ParaSET contents, they will be lost.
#
#  Restructured for paths specific to $Arch / hostname combinations.
#
# ONLY environment-persistent (setenv) settings in this file are
#   permitted. Do not use tcsh / csh "set", as these are not copied
#   to child processes and will not be seen in shells started from a
#   exceed/remote-xwin session.

# Global Environment Variables

# The latest PSETHOME
setenv PSETHOME	"`/ParaSET/tools/psethome`"

# Mount point for most shared info.
setenv SET_SHARED_DIR	/net/oxen/vol04/share

# Arch-dependent variables
setenv Arch             `/ParaSET/tools/ParaArch`
setenv ARCH		$Arch
setenv OS_SHARED_DIR    $SET_SHARED_DIR/${Arch}

# Common SET-system variables
setenv SYSBASE          /paraset
setenv BASE             $SYSBASE
setenv TOOL             $SYSBASE/tools

setenv OS               `uname -s`
setenv OSREV            `uname -r`

# UIMX variable
setenv UIMX 		/users/uimx2.0

# License configuration
setenv LM_LICENSE_FILE 7572@oxen:/users/admin/licenses_combined.electra

# Ptest variables
setenv PTEST_MASTER_LOCATION	/job1/tests/new
setenv PTEST_LOCATION	/job1/tests/$Arch
setenv PTEST_ROOT /job1/tests
setenv PTEST_HOME /ParaSET/tools

# Local storage for users
if ($?USER) then
	setenv LHOME /local_users/$USER
	if ($?HOST) then
		if ( "$HOST" == "oxen" || "$HOST" == "fridge" || "$HOST" == "bigfoot" ) then
			setenv TMPDIR $LHOME/tmpdir
			mkdir -p $TMPDIR
		endif
	endif
endif
	
# Convenience environment variables.
# -- TAKEN OUT OF SYSTEM SCRIPTS - If need be, use in local scripts
#setenv sb  $SYSBASE                 # System Base
#setenv sp  $SYSBASE/src/paraset     # System Paraset
#setenv sc  $SYSBASE/src/clients
#setenv sg  $SYSBASE/src/clients/gala/src
#setenv mb  $BASE                    # My Base
#setenv mp  $BASE/src/paraset        # My Paraset
#setenv se  $SYSBASE/bin             # System Executable
#setenv seg $SYSBASE/bin/$Arch-g    # System Executable Gee
#setenv seo $SYSBASE/bin/$Arch-O    # System Executable Ohh
#setenv me  $BASE/bin                # My Executable
#setenv meg $BASE/bin/$Arch-g         # My Executable Gee
#setenv meo $BASE/bin/$Arch-O         # My Executable Ohh
#setenv ob  /ParaSET/pset.2.1.x.JUMPED # Old System
#setenv op  $ob/src/paraset          # Old System Paraset
setenv MANPATH	/usr/local/man:/users/interviews/man:/usr/man:/usr/openwin/man:/usr/NeWSprint/man

# Definitions
#
#    PATH_person , personal directories
#    PATH_sys    , system directories which should be included in path,
#                  includes command, and X11 / OPENWIN
#    PATH_compil , optional compilers
#    PATH_bsds   , bsd-style command paths
#    PATH_local  , locally compiled utilities
#    PATH_apps   , applications and utilities like FRAME which require $VARNAME
#		   path expansions, usually NOT overridden in per-architecture section
#
# Note - ONLY PATH_compil is checked for being empty. All other variables are unconditionally
#        inserted into $PATH
#

switch ($Arch)
case "sun5":
	setenv CC_ROOTDIR	/net/oxen/vol04/SUNWspro/
	setenv CP_LANG_INCL     $CC_ROOTDIR/include/CC
        ### Set this for backward compatibility. Delete someday.
	setenv OS_ROOTDIR	$CC_ROOTDIR
	setenv PATH_person .:~/bin
	setenv PATH_sys /usr/openwin/bin:/usr/bin:/usr/ccs/bin:${SET_SHARED_DIR}/bin
	setenv PATH_compil "${CC_ROOTDIR}/bin"
	setenv PATH_bsds /usr/ucb
	setenv PATH_local /usr/local/bin:/usr/local/etc
	breaksw

case "hp700":
	setenv CC_ROOTDIR       $OS_SHARED_DIR/opt/CC
	setenv CP_LANG_INCL     $CC_ROOTDIR/include
        ### Set this for backward compatibility. Delete someday.
	setenv OS_ROOTDIR       $CC_ROOTDIR
	setenv MANPATH /usr/share/man:/opt/aCC/share/man:/opt/blinklink/share/man:/opt/langtools/share/man:/usr/contrib/man:${MANPATH}
	setenv PATH_person .:~/bin
	setenv PATH_sys /usr/bin/X11:/usr/contrib/bin/X11:/bin:/usr/bin:/usr/ccs/bin:/usr/etc:${SET_SHARED_DIR}/bin
	setenv PATH_compil /opt/aCC/bin:/opt/langtools/bin
	setenv PATH_bsds ""
	setenv PATH_local /usr/local/bin:/usr/contrib/bin
	breaksw

case "irix6":
	setenv CC_ROOTDIR	/usr
	setenv CP_LANG_INCL	$CC_ROOTDIR/include/CC
        ### Set this for backward compatibility. Delete someday.
	setenv OS_ROOTDIR       $CC_ROOTDIR
	### This for the MIPSpro compiler
	setenv LM_LICENSE_FILE	/var/flexlm/license.dat:${LM_LICENSE_FILE}
	setenv PATH_person .:~/bin
	setenv PATH_sys /usr/bin/X11:/usr/bin:/usr/sbin:/usr/etc:${SET_SHARED_DIR}/bin
	setenv PATH_compil "${CC_ROOTDIR}/bin"
	setenv PATH_bsds /usr/bsd
	setenv PATH_local /usr/local/bin
	breaksw

case "hp10":
	setenv CC_ROOTDIR /opt/aCC
	setenv CP_LANG_INCL     $CC_ROOTDIR/include
	setenv OS_ROOTDIR $CC_ROOTDIR
	setenv MANPATH /usr/share/man:/opt/aCC/share/man:/opt/blinklink/share/man:/opt/langtools/share/man:/usr/contrib/man:${MANPATH}
	setenv PATH_person .:~/bin
	setenv PATH_sys /usr/bin/X11:/usr/contrib/bin/X11:/bin:/usr/bin:/usr/ccs/bin:/usr/etc:/usr/contrib/bin:${SET_SHARED_DIR}/bin
	setenv PATH_compil ${CC_ROOTDIR}/bin:/opt/langtools/bin
	setenv PATH_bsds ""
	setenv PATH_local /usr/local/bin:/usr/contrib/bin
	breaksw

case "linux2":
	setenv CC_ROOTDIR	$OS_SHARED_DIR/KAI/KCC/KCC_BASE
	setenv CP_LANG_INCL	$CC_ROOTDIR/include
        ### Set this for backward compatibility. Delete someday.
	setenv OS_ROOTDIR       $CC_ROOTDIR
	setenv PATH_person .:~/bin
	setenv PATH_sys /usr/openwin/bin:/usr/bin/X11:/bin:/usr/bin:/usr/ccs/bin:/usr/etc:${SET_SHARED_DIR}/bin
	setenv PATH_compil ""
	setenv PATH_bsds /usr/ucb
	setenv PATH_local /usr/local/bin:/usr/local/etc
	breaksw

case "sun4":
	setenv CC_ROOTDIR	/share/opt-sparc/SWorks3.0.1
	setenv CP_LANG_INCL     $CC_ROOTDIR/include
	# set Sparcworks as default compiler:
	setenv SUN4_SPARCWORKS 1
	setenv SPRO_HOME $CC_ROOTDIR
	setenv PATH_person .:~/bin
	setenv PATH_sys /usr/openwin/bin:/usr/bin/X11:/bin:/usr/bin:/usr/ccs/bin:/usr/etc:${SET_SHARED_DIR}/bin
	setenv PATH_compil ${SPRO_HOME}:${PATH}
	setenv PATH_bsds /usr/ucb
	setenv PATH_local /usr/local/bin:/usr/local/etc
	if ( $?MANPATH ) then
		setenv MANPATH ${MANPATH}:${SPRO_HOME}/man
	else
		setenv MANPATH ${SPRO_HOME}/man
	endif
        ### Set this for backward compatibility. Delete someday.
	setenv OS_ROOTDIR       $CC_ROOTDIR
	breaksw

case default:
	echo "Unknown \$Arch type.  Exiting."
	exit 1

endsw

# Where to look for site-wide X resource definition files
setenv XAPPLRESDIR	/usr/lib/X11/app-defaults

setenv SHLIB_PATH ""
if ("$OS" != "IRIX64") then
  setenv LD_LIBRARY_PATH  $OS_ROOTDIR/lib:/usr/openwin/lib:/usr/lib
else
  setenv LD_LIBRARY_PATH ""
endif


# Purify configuration
setenv PURIFYHOME /users/purify
setenv PURIFYLOGFILE ~/purify.log
setenv PURIFYOPTIONS

# It is supposed to be a link to the current FrameMaker installation
setenv FMHOME /net/oxen/vol03/share/framemaker6.0

# The hyperhelp system
setenv HHHOME /net/oxen/vol04/share/${Arch}/bristol/hyperhelp

setenv PATH_apps ${FMHOME}/bin:/paraset/tools/ptest:/ParaSET/tools:/paraset/admin/int_tools:/users/uimx2.0/bin:/usr/atria/bin:${OS_SHARED_DIR}/bin

setenv PATH ${PATH_person}:${PATH_sys}:${PATH_compil}:${PATH_bsds}:${PATH_local}:${PATH_apps}

# Java stuff:
setenv CLASSPATH /usr/local/netscape/java_30:/usr/java/lib
