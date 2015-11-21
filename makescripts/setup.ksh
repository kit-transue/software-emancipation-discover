# $Id: setup.ksh.skeleton 1.18 2003/06/04 15:01:15EDT John Verne (jverne) Exp  $
# Quick setup script that sets up the global build environment
# for a DIScover build
#
# This is configured for the "Builder" box (win32).  
# Change accordingly for your environment
#
# Rename this to "setup.ksh", change as appropriate and source it
# in a POSIX shell to setup a CIE build environment:
#
#     . ./setup.ksh
#
# TODO:
# - Set MANPATH, at least for Solaris, to include the Sun CC man pages
# - Set PATH to SI client on those platforms that support such a thing
#

###########################################################################
# NOTE: You will probably want to change the LOCAL_SRC_ROOT and
# BUILD_SRC_ROOT (and perhaps SOLUTION_BUILD_ROOT, WEBLOGIC_HOME).  
# PATH may be tweaked for your convenience, as well.
# These settings are for the automated nightly build.
###########################################################################

#
# Misc. settings
#

# Only rebuild target jars if they don't already exist.  i.e., resuse
# jars if possible.  Set to "YES" to always force a rebuild of all jarfiles
# "NO" is the default for automated builds to speed up build times.
export REBUILDJARS=NO

export OSTYPE

#
# Host-specific settings
#
OS=`uname`

if [ "$OS" != "Windows_NT" ]
then
	# Basic PATH for all UNIX hosts for automated builds
	# Change as necessary for your environment
	# Put system paths ahead of MKS rd tools!
	PATH=/bin:/usr/bin:/usr/local/bin:/usr/rd/bin:/usr/sbin

	# Masterbuild location
	export BUILD_SRC_ROOT=/rd/integrity/buildenv/CI_7_5_0/nightly
	
	# Root of imported build tools and libraries
	# Note: this may have to change depending on where we mount
	# oxen:/vol04.  We now have a local copy, but it isn't installed
	# anywhere yet.
	BUILDENV=/net/oxen/vol04

	# To build adminui, we need a path to some imported jars
	# Change this to your local BUILD_ROOT if you like
	export SOLUTION_BUILD_ROOT=/rd/integrity/buildenv/IS_4_3_0/masterbuild

	# Same Weblogic as the version of SI we build against
	export WEBLOGIC_HOME=/rd/integrity/buildenv/IS_4_3_0/weblogic/5.1.0-sp13
else
	# Masterbuild location
	export BUILD_SRC_ROOT=D:/rd/ci_emancipation

	# Standard Win32 build tools hosted on //BUILDER/buildenv
	export BUILDENV=X:/CI_7_5_0

	# To build adminui, we need a path to some imported jars
	# Change this to your local BUILD_ROOT if you like
	export SOLUTION_BUILD_ROOT=S:/integrity/buildenv/IS_4_3_0/masterbuild

	# Same Weblogic as the version of SI we build against
	export WEBLOGIC_HOME=S:/integrity/buildenv/IS_4_3_0/weblogic/5.1.0-sp13
fi

# The full path to the local root of your build
# At some point we will support the concept of a masterbuild location
# For now, this is the same as $BUILD_SRC_ROOT
# TODO: Figure out a slick master build scenario
#export BUILD_SRC_ROOT=/rd/integrity/buildenv/CI_7_5_0/nightly
export LOCAL_SRC_ROOT=$BUILD_SRC_ROOT

# UNIX only, destination of objects and binaries
# TODO: Find a way to clean this up
export DEST=$BUILD_SRC_ROOT/dest

case $OS in
	Windows_NT)
	   TOOLS=$BUILDENV/tools

	   # MSVC-specific environment; must be exported
	   export MSVCROOT=$TOOLS/msvs60-sp4
	   export SYSCC=$MSVCROOT/VC98
	   export MSVCDir=$SYSCC
	   export INCLUDE="$SYSCC/Include;$SYSCC/MFC/Include;$SYSCC/Atl/Include;$INCLUDE"
	   export LIB="$SYSCC/Lib;$SYSCC/MFC/Lib;$LIB"

	   # Compiler tool support
	   export BISON_SIMPLE=$BUILD_SRC_ROOT/makescripts/Windows/bison.simple

	   # We require 1.4.1_01 to build adminui
	   export JAVA_HOME=$BUILDENV/jdk/1.4.1_02

	   # The full path the the directory containing the InstallShield Pro. 6.1
	   export INSTALL_HOME=$TOOLS/InstallShield6.3

	   # This has vcvars32.bat, which we need to have run prior to building
	   export PATH="$MSVCROOT/VC98/Bin;$MSVCROOT/Common/MSDev98/Bin;$PATH"

	   # Fancy titlebar for MKS Ksh users
	   # export TITLEBAR='Carbon Build - $PWD'
	   ;;

	SunOS)
	   # Solaris 2
	   PATH=$PATH:/usr/ucb:/usr/ccs/bin
	   # Location of Sun WorkShop Compilers 5.0
	   export OS_ROOTDIR=$BUILDENV/SUNWspro
	   # Some shared libraries and tools
	   export OS_SHARED_DIR=$BUILDENV/share/sun5
	   export JAVA_HOME=/project/java/1.4.1_02/sol2
	   ;;

	HP-UX)
	   # HP-UX 10 and 11
	   PATH=$PATH:/usr/ccs/bin
	   # aCC compiler and standard libraries
	   export OS_ROOTDIR=/opt
	   # Share libraries, headers and tools
	   export OS_SHARED_DIR=$BUILDENV/share/hp700-10
	   export JAVA_HOME=/project/java/1.4.1/hpux
	   ;;

	IRIX*)
	   # IRIX 6
	   PATH=$PATH:/usr/bsd
	   # Standard IRIX C++ compiler
	   export OS_ROOTDIR=/opt
	   # Share libraries, headers and tools
	   export OS_SHARED_DIR=$BUILDENV/share/irix6
	   export JAVA_HOME=/project/java/1.4.0/irix
	   ;;
esac

#EOF
