#!sh

#
# number of variables are expected to be set; they define
# location of "data", make configuration, etc.
#

MAKECVP=`pwd`

RELEASEFLAG=Debug
do_compile=yes
do_link=yes
saw_comp_list=no
comp_list=""

while [ $# -gt 0 ] ; do
	case $1 in
		Release)
			echo "Release"
			saw_comp_list=no
			RELEASEFLAG=Release;;
		Debug)
			echo "Debug"
			saw_comp_list=no
			RELEASEFLAG=Debug;;
		-nocompile)
			saw_comp_list=no
			do_compile=no;;
		-nolink)
			saw_comp_list=no
			do_link=no;;
		-comp_list)
			saw_comp_list=yes;;
		*)
			if [ $saw_comp_list = "yes" ] ; then
				comp_list="$comp_list $1"
			else
				$MAKEMSG "Invalid option $1."
				$MAKEMSG "Read make_driver.sh to see valid options."
				exit 1
			fi;;
	esac
	shift
done

LIBSTAT=0
EXESTAT=0
FAILED_LIST=""

# -- begin:

$MAKEMSG "Building [$EXEMAKE] at $EXEROOT"
$MAKEMSG "    from $MAKECVP ..."

#
# build libraries
#
if [ $do_compile = yes ] ; then
	if [ "$comp_list" != "" ] ; then
		new_list=""
		for lib in $comp_list ; do
			found=no
			for libi in $SUBPROJS ; do
				if [ "$lib" = "$libi" ] ; then
					new_list="$new_list $lib"
					found=yes
				fi
			done
			if [ "$found" = "no" ] ; then
				$MAKEMSG "Invalid library $lib ; skipping"
			fi
		done
		SUBPROJS=$new_list
	fi
	for subi in $SUBPROJS ; do
		$MAKEMSG "begin: $subi.lib ..."
		cd $MAKECVP
		cd $SUBPROJ_ROOT/$subi/make

		CFG=`$GET_CFG $RELEASEFLAG $EXEMAKE $subi`
		$MAKEMSG "CFG=$CFG"
		export CFG

		# Unconditional rebuild of version subproject
		if [ x$subi = xversion ]
		then
	   		$MAKEMSG $MAKEEXE $MAKEOPT -f $SUBPROJ_ROOT/$subi/make/$subi.mak $* CLEAN 
			$MAKEEXE $MAKEOPT -f $subi.mak $* CLEAN

	   		$MAKEMSG $MAKEEXE $MAKEOPT -f $SUBPROJ_ROOT/$subi/make/$subi.mak $* ALL
			$MAKEEXE $MAKEOPT -f $subi.mak $* ALL
		else
			$MAKEMSG $MAKEEXE $MAKEOPT -f $SUBPROJ_ROOT/$subi/make/$subi.mak $* ALL
			$MAKEEXE $MAKEOPT -f $subi.mak $* ALL
		fi
		SUBPROJ_STAT=$?
		$MAKEMSG "end:   $subi.lib ($SUBPROJ_STAT)"
		if [ $SUBPROJ_STAT != 0 ]
		then
			EXESTAT=$SUBPROJ_STAT
			FAILED_LIST="$FAILED_LIST $subi"
		fi
        done

        # build all libraries
	for libi in $LIBDIRS ; do
		$MAKEMSG "begin: $libi.lib ..."
		cd $MAKECVP
		cd $LIBROOT/$libi

		CFG=`$GET_CFG $RELEASEFLAG $EXEMAKE $libi`
		$MAKEMSG "CFG=$CFG"
		export CFG

		$MAKEMSG $MAKEEXE $MAKEOPT -f $LIBROOT/$libi/$libi.mak $* ALL
		$MAKEEXE $MAKEOPT -f $libi.mak $* ALL

		LIBSTAT=$?
		$MAKEMSG "end:   $libi.lib ($LIBSTAT)"
		if [ $LIBSTAT != 0 ]
		then
			EXESTAT=$LIBSTAT
			FAILED_LIST="$FAILED_LIST $libi"
		fi
	done
fi

cd $MAKECVP

#
# link executable
#
if [ $do_link = yes -a $EXESTAT = 0 -a "$EXEROOT" ]
then
	cd $EXEROOT

	CFG=`$GET_CFG $RELEASEFLAG $EXEMAKE`
	$MAKEMSG "CFG=${CFG}"
	export CFG

	$MAKEMSG "begin: $EXEMAKE link ..."                   
	$MAKEMSG $MAKEEXE $MAKEOPT -f $EXEROOT/$EXEMAKE.mak $* ALL
		$MAKEEXE $MAKEOPT -f $EXEMAKE.mak $*  ALL
		EXESTAT=$?
		if [ $EXESTAT != 0 ] ; then
			FAILED_LIST="$FAILED_LIST $EXEMAKE"
		fi
	$MAKEMSG "end:   $EXEMAKE ($EXESTAT)"
fi

cd $MAKECVP

# -- end:

#
# status information
#
if [ $EXESTAT != 0 ] ; then
	$MAKEMSG "STATUS: $EXEMAKE Failed ($EXESTAT)"
	$MAKEMSG "Failed list: $FAILED_LIST"
else
	$MAKEMSG "STATUS: $EXEMAKE OK"
fi
