#!/bin/ksh
# Use the above hashbang for building on HP
#!/usr/rd/bin/ksh

#
# build all relevant executables
#

MAJOREXELIST="aset:model_server \
	paracancel \
	model_server:model_server \
	dis_sql \
	simplify \
	mrg_diff \
	mrg_ui \
	mrg_update \
	aset_CCcc \
	ifext \
	disperl \
	dismb\
	checklist \
	dis_path \
	aset_jfe \
	sevreporter \
	pdfgen \
	scanclrmake \
	parsercmd \
	log_presentation \
	cmdriver \
	mbdriver \
	cmdish \
	ci_environment \
    	ciadmin:adminui"

MINOREXELIST="admintool \
    clearcase:config/config \
    config \
    configshell:config/configshell \
    configurator:config/configurator \
    continuus:config/config \
    cvs:config/config \
    DevXLauncher \
    diff \
    discrypt \
    dish2:dish \
    EditorDrivers:emacs_integration \
    flex \
    lock \
    nameserv:nameserver \
    rcs:config/config \
    sccs:config/config \
    serverspy \
    tclsh:dis_tcl8.3 \
    vim:vim-5.3 \
    wish:dis_tk8.3 \
    xutil \
    devxAdapter \
    emacsAdapter \
    md5"

ENV_VARS="OS_ROOTDIR \
	OS_SHARED_DIR \
	BUILD_SRC_ROOT \
	LOCAL_SRC_ROOT"

MAKEMSG='echo'

#
# build executables
#

BUILDMAJOR=
BUILDMINOR=
opts=$*
for opt in ${opts}
do
	case ${opt} in
	-minor)
		BUILDMINOR="1";;
	-major)
		BUILDMAJOR="1";;
	-all)
		BUILDMAJOR="1"
		BUILDMINOR="1";;
	esac
done

for envvar in ${ENV_VARS}
do
	ENV_VAL=`eval echo \\$$envvar`
	if [ -z ${ENV_VAL} ] ; then
		$MAKEMSG "Please define ${envvar} variable"
		BUILDMAJOR=""
		BUILDMINOR=""
	fi
done

if [ -z $BUILDMINOR$BUILDMAJOR ] ; then
	exit
fi

BUILDTIME=`date '+%a %b %e %T %Z %Y'`
$MAKEMSG "Build started $BUILDTIME..."

LOGDIR=${BUILD_SRC_ROOT}/buildlogs
if [ ! -d ${LOGDIR} ] ; then
        mkdir ${LOGDIR}
fi

if [ -d ${LOCAL_SRC_ROOT}/makescripts/Unix ]; then
    SCRIPT_DIR=${LOCAL_SRC_ROOT}/makescripts/Unix
elif [ -d ${BUILD_SRC_ROOT}/makescripts/Unix ]; then
    SCRIPT_DIR=${BUILD_SRC_ROOT}/makescripts/Unix
fi

$MAKEMSG "Script dir is ${SCRIPT_DIR}"
. ${SCRIPT_DIR}/set_arch.sh

if [ ! -z ${BUILDMAJOR} ] ; then
    $MAKEMSG "Building major projects..."
    for exei in $MAJOREXELIST
	do
	CURDIR=`pwd`
	BUILDDIR=${exei#*:} # trying to get directory name 
		        # it should be separated by ':' from project name
	if [ -z ${BUILDDIR} ] ; then # if directory name is not specified - assume that 
				 # a project name and a directory name are the same
	    BUILDDIR=${exei}
	fi
	cd ${BUILD_SRC_ROOT}/${BUILDDIR}
	exei=${exei%%:*} # get a project name
	$MAKEMSG "Integration of $exei ..."
	LOGFILE=${LOGDIR}/${exei}_${ARCH}.buildlog
	if [ -f "${LOGFILE}" ] ; then
	    $MAKEMSG "Removing a previous build log..."
	    rm ${LOGFILE}
	fi
	$MAKEMSG "Building $exei ..."
	./make.$exei.sh > ${LOGFILE} 2>&1
	EXESTAT=$?
	if [ "${EXESTAT}" = "0" ]; then
	    $MAKEMSG "Done ($EXESTAT)"
	else
		$MAKEMSG "FAILED ($EXESTAT): see ${LOGFILE} for details."
        fi			
	cd ${CURDIR}
    done
fi
    
if [ ! -z ${BUILDMINOR} ] ; then
    $MAKEMSG "Building minor projects..."

    . ${SCRIPT_DIR}/make_exec.cf
    export MAKELIB OBJ_ROOT BUILD_SRC_ROOT ARCH Arch TOOLDIR

    MAKE=${GMAKE}

    for exei in $MINOREXELIST
    do
        CURDIR=`pwd`
        BUILDDIR=${exei#*:} # trying to get directory name 
    		        # it should be separated by ':' from project name
        if [ -z ${BUILDDIR} ] ; then # if directory name is not specified - assume that 
    				 # a project name and a directory name are the same
		BUILDDIR=${exei}
        fi
        cd ${BUILD_SRC_ROOT}/${BUILDDIR}
        exei=${exei%%:*} # get a project name

        $MAKEMSG "Integration of $exei ..."
        LOGFILE=${LOGDIR}/${exei}_${ARCH}.buildlog
        if [ -f "${LOGFILE}" ] ; then
		$MAKEMSG "Removing a previous build log..."
		rm ${LOGFILE}
        fi
        $MAKEMSG "Building $exei ..."
        ${MAKE} -f ${MAKELIB}/Makefile_minor $exei > ${LOGFILE} 2>&1
        EXESTAT=$?
        if [ "${EXESTAT}" = "0" ]; then
		$MAKEMSG "Done ($EXESTAT)"
        else
		$MAKEMSG "FAILED ($EXESTAT): see ${LOGFILE} for details."
        fi			
        cd ${CURDIR}
    done
fi
BUILDTIME=`date '+%a %b %e %T %Z %Y'`
$MAKEMSG "Build finished $BUILDTIME."

