#!sh

#
# location of the source tree, executable, and the name of executable
#
EXEROOT='.'
EXEMAKE='dish2'

#
#
# list of libraries that make dish2
#
LIBROOT='libs'
LIBDIRS="\
	nameserver \
	src \
	xxinterface"

 
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
