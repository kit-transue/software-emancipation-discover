#!sh

#
# location of the source tree, executable, and the name of executable
#
EXEROOT='.'
EXEMAKE='aset_jfe'

#
# list of libraries that make EDG jfe parser
#
LIBROOT='libs'
LIBDIRS="\
	md5 \
	nameserver \
	stream_message \
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
