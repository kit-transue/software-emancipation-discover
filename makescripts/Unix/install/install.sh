#!/bin/sh
# install.sh

# creates installation after a build.
# must be run in the same view as the build (same config_spec plus derived objects)

PROJ_ROOT=${BUILD_SRC_ROOT}
export PROJ_ROOT

. ${PROJ_ROOT}/makescripts/Unix/install/build_support.sh
. ${PROJ_ROOT}/makescripts/Unix/set_arch.sh

DISPLAY=""
RUN_TESTS=0
while [ $# -gt 0 ]
do
   case $1 in
	-display)
		DISPLAY="-display $2"
		shift 2
		;;
	motif)
		TARGET=DISCOVER
		INSTALL_NAME=codeintegrity
		shift
		;;
	run_tests)	
		RUN_TESTS=1
		shift
		;;
	java)
		TARGET=CODEROVER
		INSTALL_NAME=coderover
		shift
		;;
	*)
		echo "install.sh: Unknown option $1.  Exiting."
		exit 1
		;;
   esac
done

version=`cat ${VERSION_FILE}`

INSTALL_DIR="${ARCH_INSTALL_BASE}/${version}/${ARCH}/${INSTALL_NAME}"

mkdir -p ${INSTALL_DIR}
cd ${INSTALL_DIR}
${PROJ_ROOT}/makescripts/bin/${Arch}/gmake ARCHV=${ARCHV} -f ${PROJ_ROOT}/makescripts/Unix/install/Makefile ${TARGET}

if [ $? -ne 0 ] ; then
	echo "Install image failed; skipping installer package and regression tests."
	exit 1
else
	# Make the final distributable
	# THIS ONLY WORKS FOR DISCOVER/DIScover
	echo "Making final distributable"
	OPWD=$PWD
	cd "${ARCH_INSTALL_BASE}/${version}/${ARCH}"

	MKSELF=${PROJ_ROOT}/makescripts/Unix/install/makeself/makeself.sh
	MKSELFARGS="--compress --notemp"
	DIST_NAME=${INSTALL_NAME}.bin
	${MKSELF} ${MKSELFARGS} ${INSTALL_NAME} ${DIST_NAME} \
	'DIScover' ./setup

	cd $OPWD

	# Run regressions, if necessary
	if [ ${RUN_TESTS} -eq 1 ] ; then
		${PROJ_ROOT}/makescripts/Unix/install/run_system_tests.sh -psethome `pwd` $DISPLAY
	fi
fi

