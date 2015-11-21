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
# usage: testct { <testdir> } { <source> }
#
# NOTE: use absolute pathnames when specifying <testdir>, and/or <source>
#

TESTDIR="$1"
if [ "$TESTDIR" = "" ] ; then
	TESTDIR=testedg
fi

SRCDIR="$2"
if [ "$SRCDIR" = "" ] ; then
	SRCDIR="`pwd`"
fi

CURDIR="`pwd`"

#
# -- setup parser environment
#
PARSER=aset_CCcc
if [ "$PARSER" = "" ] ; then
	echo "Could not locate aset_CCcc"
	SRCDIR=
else
	mkdir -p $TESTDIR
	mkdir -p $TESTDIR/source
	mkdir -p $TESTDIR/base
	mkdir -p $TESTDIR/log
	if [ ! -d "$TESTDIR/log" ] ; then
		echo "Could not create test directories at $TESTDIR"
		SRCDIR=
	fi
fi
CFLAGS="-w"

if [ -d "$SRCDIR" ] ; then
	echo " "
	echo "Building test directory $TESTDIR for"
	echo "  source code at $SRCDIR"
	echo " "
	# -- files to compile
	cd $SRCDIR
	SRCLIST="`find . -name \"*.[cC]\" -print`"
	for file in $SRCLIST
	do
		echo "Building $file.iff ..."
		$PARSER $CFLAGS --IF_file $file.iff $file > $TESTDIR/log/$file.run 2>&1
		if [ -f $SRCDIR/$file.iff ] ; then
			cp $file $TESTDIR/source
			chmod +w $TESTDIR/source/$file
			cp $file.iff $TESTDIR/base/$file
			rm -f $file.iff
		else
			echo "+: Could not build $file"
			cat $TESTDIR/log/$file.run
			echo "-:"
		fi
		rm -f $TESTDIR/log/$file.run
	done
else
	echo "Could not find source code"
fi
echo "Done"

cd $CURDIR
