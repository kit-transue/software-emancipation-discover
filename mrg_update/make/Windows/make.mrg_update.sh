#!sh

#
# location of the source tree, executable, and the name of executable
#
LIBROOT='.'
EXEROOT='.'
EXEMAKE='mrg_update'

#
# list of libraries that make mrg_update
#
LIBROOT='libs'
LIBDIRS="\
	update \
	machine \
	gen \
	nihcl \
	nameserver \
	stream_message"


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
