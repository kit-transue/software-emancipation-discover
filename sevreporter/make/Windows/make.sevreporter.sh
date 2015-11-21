#!sh

#
# location of the source tree, executable, and the name of executable
#
EXEROOT='.'
EXEMAKE='sevreporter'

#
# list of libraries that make sevreporter
#
LIBROOT='libs'
LIBDIRS="\
	source \
	osport \
	gdchart \
	gd \
	ads"

#
# -- begin: invoke the actual build
#
ADMMAKE=`pwd`/../makescripts/Windows
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
