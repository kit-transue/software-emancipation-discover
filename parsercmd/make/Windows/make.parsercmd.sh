#!sh

#
# location of the source tree, executable, and the name of executable
#
EXEROOT='.'
EXEMAKE='parsercmd'

#
# list of libraries that make dish2
#
LIBROOT='libs'
LIBDIRS="nameserver\
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
