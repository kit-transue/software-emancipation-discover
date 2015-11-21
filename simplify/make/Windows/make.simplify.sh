#!sh

#
# location of the source tree, executable, and the name of executable
#
EXEROOT='.'
EXEMAKE='simplify'

#
# list of libraries that make simplify
#
LIBROOT='libs'
LIBDIRS="source \
	machine \
	osport \
	if_parser \
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
