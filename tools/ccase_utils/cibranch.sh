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
# Comment
#

my_name=`basename $0`
ct=cleartool
logfile=/users/sschmidt/ccase_utils/${my_name}.log

Err ()
{
	echo $* 1>&2
}

Usage ()
{
	Err "Usage: $my_name args"
	Err "       e.g. $my_name"
	now=`date`
	echo "$now : $my_name failed for $USER" >> $logfile
	exit 1
}

Calculate_Tag ()
{
	if [ "$tag" = "" ] ; then
		tmpfile=${TMPDIR}/${my_name}.tmp.$$

		$ct catcs > $tmpfile
		line=`grep "main/.*/LATEST" $tmpfile | head -1`
		tag=`echo $line | sed 's#^element.*/main/##g' | sed 's#/LATEST.*##'`

		rm -f $tmpfile
	else
		$ct lsview $tag > /dev/null 2>&1
		if [ $? -ne 0 ] ; then
			Err "Specified view $tag does not exist"
			Usage
		fi
	fi
}

now=`date`
echo "$now : ${my_name} run with args $* by $USER" >> $logfile

tag=""
view_store=/net/oxen/vol02/views
comment="-nc"
brtype=""
bug_number=""

while [ $# -gt 0 ] ; do
	case $1 in
		-tag)
			shift ; tag=$1;;
		*)
			Err "Invalid argument $1"
			Usage;;
	esac
	shift
done

Calculate_Tag

Calculate_Vobs

Make_Branch

now=`date`
echo "$now : ${my_name} succeeded for $USER" >> $logfile
