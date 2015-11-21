#!sh
#
# location of the source tree, executable, and the name of executable
#
EXEROOT='Windows'
EXEMAKE='pset_server'

#
# location of the source tree and list of subprojects that make model_server
#
SUBPROJ_ROOT='..'
SUBPROJS="\
        DIS_cancel \
        DIS_cm \
        DIS_create \
        DIS_dormant \
        DIS_main \
        DIS_pset \
        DIS_rtlFilter \
        DIS_ui \
        IDE_editor \
        OODT \
        api \
        api_newui \
        api_pset \
        assoc \
        ast \
        dagraph \
        dagraph_disgui \
        dagraph_dispmod \
	dfa \
	dd \
	driver \
	driver_aset \
	driver_mb \
        edge \
	els \
        gala \
	gala_communications \
	gala_machine \
        gedge \
        graGala \
	interface \
	lde \
	ldr \
	machine_prefs \
	obj \
	partition \
	process \
	project_cm \
	project \
	project_file \
	project_pdf \
	project_ui \
	report \
	rtl \
	save \
	search \
	smt \
	smt_metric \
	ste \
	ste_disstub \
	style \
	subsystem \
	test \
	to_cpp \
	util \
	view \
	view_disstub \
        view_rtl \
	version \
	xref"

#
# location and list of libraries that model_server uses
#
LIBROOT='../libs'
LIBDIRS="\
        ads \
        gen \
        if_parser \
        nameserver \
        nihcl \
        osport \
        stream_message \
        xxinterface"

#
# -- begin: invoke the actual build
#

#
#Set location of makescripts to ADMMAKE variable. If makescripts project
#does not exist locally then use the build sandbox location
#
if [ -d "../../makescripts/Windows" ] ; then
    ADMMAKE=`pwd`/../../makescripts/Windows
else
    ADMMAKE="Q:/makescripts/Windows"
fi
export ADMMAKE


#
# configure make (default)
#

. $ADMMAKE/make_config.sh

#
# start the driver
#
. $ADMMAKE/make_driver.sh $*

#
# -- end:
#
