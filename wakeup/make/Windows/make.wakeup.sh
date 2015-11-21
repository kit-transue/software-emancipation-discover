#!sh

#
# location of the source tree, executable, and the name of executable
#
LIBROOT=
EXEROOT='.'
EXEMAKE='wakeup'

#
# list of libraries that make EDG jfe parser
#
LIBDIRS=

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
