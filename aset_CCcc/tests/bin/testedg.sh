#!/bin/sh -f
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
# -- TESTDIR is the location where ./base, ./source, and ./log
#    directories are expected
#
# 	usage: testedg { <directory> } { <aset_CCcc> }
#
# NOTE: use absolute pathnames when specifying <directory>, and/or <aset_CCcc>
#

TESTDIR="$1"
if [ "$TESTDIR" = "" ] ; then
	TESTDIR="`pwd`"
fi

PARSER="$2"
if [ "$PARSER" = "" ] ; then
	PARSER=aset_CCcc
fi

#
# -- setup necessary environment
#
CURDIR="`pwd`"

# --
CMPDIR="$TESTDIR/base"
SRCDIR="$TESTDIR/source"
LOGDIR="$TESTDIR/log"
CFLAGS="-w"

# -- build the list of files to validate
SRCLST=

cd $SRCDIR
for file in `find . -name \"*.[cC]\" -print`
do
	cmpfile="$CMPDIR/$file"
	if [ -f "$cmpfile" ] ; then
		SRCLST="$SRCLST $file"
	fi
done

# -- run the parser on the selected list of files
echo "`date`: Testing Parser \"$PARSER\" at $TESTDIR ..."
cd $LOGDIR
for file in $SRCLST
do
	runfile="FAILED"
	#
	# -- need to be in the source directory in order to match IF contents
	#
	(cd $SRCDIR; $PARSER $CFLAGS --IF_file $LOGDIR/$file.cmp $file > $LOGDIR/$file.run 2>&1)
	#
	# -- process the generated output
	#
	if [ -f $file.cmp ] ; then
		# -- compare files
		diff $file.cmp $CMPDIR/$file > $file.diff 2>&1
		rundiff=`wc -l $file.diff | sed 's/[^0-9]*//' | sed 's/ .*//'`
		if [ "$rundiff" = "0" ] ; then
			runfile="OK    "
		fi
	fi
	echo "$runfile: $file"
	if [ "$runfile" = "FAILED" ] ; then
		echo "   ======== $file.cmp vs. base"
		cat $file.diff
		echo "   ======== execution output"
		cat $file.run
		echo " "
	fi
	rm -f $file.diff $file.cmp $file.run
done
echo "`date`: Done"

cd $CURDIR
