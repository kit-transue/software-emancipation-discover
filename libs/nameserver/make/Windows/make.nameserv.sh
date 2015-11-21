#!sh

#
# location of the source tree, executable, and the name of executable
#
LIBROOT=
EXEROOT='.'
EXEMAKE='nameserver'

#
# list of libraries that make nameserv
#
LIBDIRS=

#
# -- begin: invoke the actual build
#
ADMMAKE='../makescripts/Windows'

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
