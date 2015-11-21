#!sh

#
# location of the executable, and the name of executable
#
EXEROOT='Windows'
EXEMAKE='model_server'

#
# location of the source tree and list of subprojects that make model_server
#
SUBPROJ_ROOT='..'
SUBPROJS="\
	OODT \
	api \
	api_pset \
	assoc \
	ast \
	dd \
	debug \
	dfa \
	driver \
	driver_model_server \
	driver_mb \
	els \
	machine_prefs \
	obj \
	process \
	project \
	project_file \
	project_pdf \
	rtl \
	save \
	search \
	smt \
	smt_metric \
	to_cpp \
	util \
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
	machine \
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
