#!sh

#
# location of the source tree, executable, and the name of executable
#
EXEROOT='.'
EXEMAKE='ifext'

#
# list of libraries that make dis_sql
#
LIBROOT='libs'
LIBDIRS="nihcl \
	gen \
	nameserver\
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
