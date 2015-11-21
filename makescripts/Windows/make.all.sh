#!/bin/ksh

#
# build all relevant executables
#

EXELIST=`cat ./win_projects.lst` 

MAKEMSG='echo'

#
# build executables
#

BUILDROOT=${BUILD_SRC_ROOT}

BUILDTIME=`date '+%a %b %e %T %Z %Y'`
$MAKEMSG "Build started $BUILDTIME..."
vcvars32
for exei in $EXELIST
	do
		CURDIR=`pwd`
		# trying to get directory name 
		# it should be separated by ':' from project name
		BUILDDIR=${exei#*:}

	       	# if directory name is not specified - assume that 
		# a project name and a directory name are the same
		if [ -z ${BUILDDIR} ] ;
	       	then
			BUILDDIR=${exei}
		fi
		cd ${BUILDROOT}/${BUILDDIR}
		exei=${exei%%:*} # get a project name

		$MAKEMSG "Integration of $exei ..."
		if [ -f "${exei}.buildlog" ] ; then
			$MAKEMSG "Removing a previous build log..."
			chmod a+w $exei.buildlog
			rm $exei.buildlog
		fi
		$MAKEMSG "Building $exei ..."
		./make.$exei.sh $* > $exei.buildlog 2>&1
		EXESTAT=$?
		if [ "${EXESTAT}" = "0" ]; then
			$MAKEMSG "Done ($EXESTAT)"
		else
			$MAKEMSG "FAILED ($EXESTAT): see `pwd`/$exei.buildlog for details."
		fi			
		cd ${CURDIR}
	done
BUILDTIME=`date '+%a %b %e %T %Z %Y'`
$MAKEMSG "Build finished $BUILDTIME."
