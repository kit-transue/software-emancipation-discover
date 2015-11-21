#!/bin/sh
##########################################################################
# Copyright (c) 2015, Synopsys, Inc.                                     #
# All rights reserved.                                                   #
#                                                                        #
# Redistribution and use in source and binary forms, with or without     #
# modification, are permitted provided that the following conditions are #
# met:                                                                   #
#                                                                        #
# 1. Redistributions of source code must retain the above copyright      #
# notice, this list of conditions and the following disclaimer.          #
#                                                                        #
# 2. Redistributions in binary form must reproduce the above copyright   #
# notice, this list of conditions and the following disclaimer in the    #
# documentation and/or other materials provided with the distribution.   #
#                                                                        #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    #
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      #
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  #
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   #
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, #
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       #
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  #
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  #
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    #
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  #
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   #
##########################################################################
#
# copy compile areas to master directory and switch 
# 	install.stable  -> install.old 
#	install.new     -> install.stable
#	install.new.tmp -> install.new	
#	
#
# Synopsis --
#	release <PLATFORM> <TIMEOUT>
# 
# needs to be done on electra (gtar not available on hp/solaris)
#

HOST=$1
MSG=/paraset/tools/builder
TIMEOUT=${2:-5}
FROM=/$HOST-compile/release/$HOST/install.new.tmp
TO=/compile/release/$HOST/install.new.tmp
INSTALL=/compile/release/$HOST/install
SUPPORT=/net/electra/local1b/nexus/Releases/$HOST/latest.tmp/paraset
export HOST TIMEOUT FROM TO SUPPORT

echo "*****msg: $MSG TIMEOUT: $TIMEOUT FROM: $FROM TO: $TO INSTALL: $INSTALL******"

HERE=`pwd`

if [ `uname -n` != electra ] ; then
	echo " Installations should be done only from electra"
	echo " You are currently on `uname -n`"
	echo "               exiting"
	exit 1
fi

[ -d $INSTALL.old ] && rm -rf  $INSTALL.old

[ ! -d $FROM ] && exit 1
[ ! -d $TO ] && mkdir -p $TO

cd $FROM
gtar -cf - . | (cd $TO ; gtar -xBpf - ) 

[ ! $? ] && exit 1

/usr/ucb/mail -s " Release of installation for $HOST" aset-devlop <<EOF

	Hello,
		$HOST  executables will be installed in $TIMEOUT MINUTES.
	please save your session  by then. If you dont want to listen
	to broadcast messges set mesg n. You may want to look at mesg
	manual page.

	Thank you,
	Builder
	P.S: Please delete this mail after you have read it.
EOF

t=$TIMEOUT
while [ $t  -gt 0 ]
do
	#sed "s/TIMEOUT/$t/" $MSG/timeout | sed "s/HOST/$HOST/" >   /tmp/$$.timeout
  	#/tmp/$$.timeout
	[ $t -eq $TIMEOUT ] && NOMSG="If you want to ignore future broadcasts set: mesg n"
	[ $t -eq `expr $TIMEOUT - 1 ` ] && NOMSG="If you want to ignore future broadcasts set: mesg n"
	if [ $t -gt 1 ] ; then
		min=minutes
	else
		min=minute
	fi
	rwall `cat /paraset/tools/builder/hostlist` <<EOF &

	 $t $min remaining before $HOST installation begins.
	 $NOMSG

EOF
	sleep `expr $t '*' 30`
	t=`expr $t - $t / 2 - 1 `
done 


echo -n '---------------------------> continue? '
read c
[ -d $INSTALL.stable ] && mv $INSTALL.stable $INSTALL.old
[ -d $INSTALL.new ] && mv $INSTALL.new $INSTALL.stable
[ -d $INSTALL.new.tmp ] && mv $INSTALL.new.tmp $INSTALL.new
rm -rf $INSTALL.stable/${HOST}-g

touch $INSTALL.new

rwall `cat /paraset/tools/builder/hostlist` <<EOF &

		$HOST Installation has finished..
EOF

[ -f /tmp/$$.timeout.1 ] && rm -rf /tmp/$$.timeout.1
[ -f /tmp/$$.timeout ]   && rm -rf /tmp/$$.timeout

exit 0
#
# Copy stuff to support machines
#

echo " Copying stuff  for support...onto $SUPPORT"

mkdir -p $SUPPORT
cd  $INSTALL.stable/${HOST}-O
gtar -cf - . | (cd $SUPPORT; gtar -xBpf - ) 
rm $SUPPORT/bin/*pure*
rm $SUPPORT/bin/aset.dir
rm $SUPPORT/bin/aset.pag
mv $SUPPORT/bin/aset $SUPPORT/bin/aset.mixed
mv $SUPPORT/bin/aset.install $SUPPORT/bin/aset

/usr/ucb/mail -s " Release of installation for $HOST" supportg <<EOF

	Hello,
		$HOST  executables will be installed in $TIMEOUT MINUTES.
	please save your session  by then. If you dont want to listen
	to broadcast messges set mesg n. You may want to look at mesg
	manual page.

	Thank you,
	Builder
	P.S: Please delete this mail after you have read it.
EOF

sleep `expr $TIMEOUT '*' 60`

[ -d $SUPPORT/../../latest/paraset.old ] && rm -rf $SUPPORT/../../latest/paraset.old 
[ -d $SUPPORT/../../latest/paraset ] && mv $SUPPORT/../../latest/paraset $SUPPORT/../../latest/paraset.old
mv $SUPPORT $SUPPORT/../../latest

cd $HERE

