#!sh

if [ -d '../dis_tcl8.3' ] ; then 
	cd ../dis_tcl8.3
	./make.tcl.sh
	cd ../mrg_ui
fi

if [ -d '../dis_tk8.3' ] ; then 
	cd ../dis_tk8.3
	./make.tk.sh
	cd ../mrg_ui
fi


#
# location of the source tree, executable, and the name of executable
#
EXEROOT='.'
EXEMAKE='mrg_ui'

#
# list of libraries that make mrg_ui
#
LIBROOT='libs'
LIBDIRS="mtk \
	machine \
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
