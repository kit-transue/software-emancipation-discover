#!/bin/ksh

# For use with the nightly build process.  If LOCAL_SRC_ROOT is empty we 
# haven't sourced setup.ksh yet, so this hard code job shouldn't afect
# developers.
ENV_VAL=`eval echo $LOCAL_SRC_ROOT`
echo ${ENV_VAL}
if [ -z "${ENV_VAL}" ] ; then
	export LOCAL_SRC_ROOT=/rd/integrity/buildenv/CI_7_5_0/nightly
	. $LOCAL_SRC_ROOT/makescripts/setup.ksh
fi

PROJ_LIST="paracancel \
	model_server \
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
	cmdish\
	admintool \
	config \
	DevXLauncher \
	diff \
	discrypt \
	dish \
	emacs_integration \
	flex \
	lock \
	nameserver \
	serverspy \
	dis_tcl8.3 \
	vim-5.3 \
	dis_tk8.3 \
	xutil \
	devxAdapter \
	emacsAdapter \
	gala \
	stream_message \
	machine \
	gen \
	nihcl \
	xxinterface \
	FLEXlm \
	editorinterface \
	els \
	gt \
	gtxm \
	impact \
	md5 \
	ci_environment \
	template_dictionary \
	xpm-3.4k\
	adminui"

MAKEMSG='echo'
DO_INSTALL=0
DO_BUILD=0
DO_RESYNC=0
RUN_TESTS=
DISPLAY=
while [ $# -gt 0 ]
do
	case $1 in
		-display)
			DISPLAY="-display $2"
			shift 2;;
		-run_tests)
			RUN_TESTS=run_tests
			shift;;
		-install)
			DO_INSTALL=1
			shift;;
		-build)
			DO_BUILD=1
			shift;;
		-resync)
			DO_RESYNC=1
			shift;;
		*)
			$MAKEMSG "Illegal option: $1"
			shift;; 
	esac
done

# TODO: fix this
PROJ_ROOT=/net/masunfs1/si/Projects
BUILD_HOST=masunfs1
if [ ${DO_RESYNC} -eq 1 ] ; then
	$MAKEMSG "Resynchronizing projects..."
	for proj in $PROJ_LIST
	do
		rsh ${BUILD_HOST}  si resync -R -S ${PROJ_ROOT}/${proj}/${proj}.pj
	done
fi

cd ${BUILD_SRC_ROOT}/makescripts/Unix/

# We can probably stop doing this, as we can force the executable bit on
# via an SI attribute
$MAKEMSG "Making scripts executable..."
chmod +x `cat executables.list`

if [ ${DO_BUILD} -eq 1 ] ; then
        $MAKEMSG "Building executables..."
	${BUILD_SRC_ROOT}/makescripts/Unix/make.all.sh -all
fi

if [ ${DO_INSTALL} -eq 1 ] ; then
        $MAKEMSG "Creating install image..."
	${BUILD_SRC_ROOT}/makescripts/Unix/install/install.sh motif ${RUN_TESTS} ${DISPLAY}
fi
