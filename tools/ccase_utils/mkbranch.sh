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
# This script makes a branch type in the /paraset vob, create a view
# with the same name as the branch type, set the config spec of the
# newly created view appropriately, and return the name of the view.
# The view will be stored in /net/oxen/vol02/views.  The calculation of
# the appropriate name for the branch and view is somewhat involved.
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
	Err "Usage: $my_name <bug_number|-tag tag> [-view_store view_storage_location] [-vobs list_of_vobs]"
	Err "       e.g. $my_name 19112 -view_store /net/oxen/vol02/views -vobs /paraset /job1"
	now=`date`
	echo "$now : ${my_name} failed for $USER" >> $logfile
	exit 1
}

Check_Vobs ()
{
	if [ "$vobs" = "" ] ; then
		Usage
	else
		for vob in $vobs ; do
			$ct desc -vob $vob > /dev/null 2>&1
			if [ $? -ne 0 ] ; then
				Err "Invalid VOB $vob specified."
				Usage
			else
				if [ ! -d $vob ] ; then
					Err "Valid VOB $vob not accessible?!"
					Usage
				fi
			fi
		done
	fi
}

Check_Tag_As_View ()
{
	$ct lsview $tag > /dev/null 2>&1
	if [ $? -eq 0 ] ; then
		return 1
	else
		return 0
	fi
}

Check_Tag_In_Vob ()
{
#
# Returns 0 if no branch with name $tag exists in vob $1
# Returns 1 if a branch with name $tag exists in vob $1 and is locked
# Returns 2 if a branch with name $tag exists in vob $1 and is not locked
#
	cd $1
	descr=`$ct desc brtype:$tag 2>/dev/null | grep "branch type"`
	if [ "$descr" = "" ] ; then
		return 0
	else
		echo $descr | grep locked > /dev/null 2>&1
		if [ $? -eq 0 ] ; then
			return 1
		else
			return 2
		fi
	fi
}

Try_Tag ()
{
	for vob in $vobs ; do
		Check_Tag_In_Vob $vob
		val=$?
		if [ $val -eq 2 ] ; then
			Err "Calculated branch $tag is currently active in VOB $vob."
			Err "Either specify a tag with -tag, or use the currently"
			Err "active branch and view for bug $bug_number."
			Usage
		elif [ $val -eq 1 ] ; then
			return 1
		fi
	done
	Check_Tag_As_View
	return $?
}

Calculate_Tag ()
{
	if [ "$tag" = "" ] ; then
		if [ "$bug_number" = "" ] ; then
			Err "Must specify a tag with -tag or a bug number."
			Usage
		fi
		tag="bug$bug_number"
		Try_Tag
		if [ $? -ne 0 ] ; then
			tag="$USER.bug$bug_number"
			Try_Tag
			if [ $? -ne 0 ] ; then
				numlist="1 2 3 4 5 6 7 8 9 10"
				for num in $numlist ; do
					tag="bug$bug_number.$num"
					Try_Tag
					if [ $? -eq 0 ] ; then
						break
					fi
				done
			fi
		fi
		Try_Tag
		if [ $? -ne 0 ] ; then
			Err "Calculated branches bug$bug_number, $USER.bug$bug_number,"
			Err "and bug$bug_number.1 through 10 already exist and are"
			Err "locked, or views with these names already exist."
			Err "This certainly must be a difficult bug :)"
			Err "Specify a tag with -tag."
			Usage
		fi
	else
		for vob in $vobs ; do
			Check_Tag_In_Vob $vob
			if [ $? -ne 0 ] ; then
				Err "Branch $tag already exists in VOB $vob."
				Usage
			fi
		done
		Check_Tag_As_View
		if [ $? -ne 0 ] ; then
			Err "View $tag already exists."
			Usage
		fi
	fi
}

Make_Branch ()
{
	for vob in $vobs ; do
		cd $vob
		$ct mkbrtype $comment $tag >> $logfile 2>&1
	done
}

Make_View ()
{
	$ct mkview -tag $tag ${view_store}/${tag}.vws >> $logfile 2>&1
}

Set_ConfigSpec ()
{
	tmpfile=${TMPDIR}/${my_name}.$$
	echo "element * CHECKEDOUT" > $tmpfile
	echo "element * /main/${tag}/LATEST" >> $tmpfile
	echo "element * /main/LATEST -mkbranch ${tag}" >> $tmpfile
	echo "Config spec for view $tag :" >> $logfile
	cat $tmpfile >> $logfile
	$ct setcs -tag $tag $tmpfile
	rm -f $tmpfile
}

now=`date`
echo "$now : ${my_name} run with args $* by $USER" >> $logfile

tag=""
view_store=/net/oxen/vol02/views
comment="-nc"
vobs=/paraset
brtype=""
bug_number=""

while [ $# -gt 0 ] ; do
	case $1 in
		-tag)
			shift ; tag=$1;;
		-view_store)
			shift ; view_store=$1;;
		-nc*)
			comment="-nc";;
		-vobs)
			vobs=""
			while [ $# -gt 1 ] ; do
				case $2 in
					-*)
						break;;
					*)
						vobs="$vobs $2"; shift;;
				esac
			done;;
		-*)
			Err "Invalid argument $1"
			Usage;;
		*)
			bug_number=$1;;
	esac
	shift
done

Check_Vobs

Calculate_Tag

Make_Branch

Make_View

Set_ConfigSpec

echo "$tag"

now=`date`
echo "$now : ${my_name} succeeded for $USER" >> $logfile
