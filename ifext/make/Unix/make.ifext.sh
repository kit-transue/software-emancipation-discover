#!/bin/sh

targ=ifext

curDir=`pwd`

SCRIPT_DIR=${LOCAL_SRC_ROOT}/makescripts/Unix
if [ ! -d ${SCRIPT_DIR} ]
then 
	SCRIPT_DIR=${BUILD_SRC_ROOT}/makescripts/Unix
fi
cd ${SCRIPT_DIR}
. ./ma
cd "${curDir}"
