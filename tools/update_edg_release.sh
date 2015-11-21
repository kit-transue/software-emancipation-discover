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
# update_edg_release.sh
# add edg release to branch; create label

if [ $# != 2 ]; then
	echo "usage: $0 /fullpath/to/directories-with-sources version" >& 2
	echo "	E.G.: $0 /users/tmp/edgcpfe_2.37/release_2.38/src 2.38_special" >& 2
	echo "	E.G.: $0 /net/incubus/sunny/edg.2.43/src 2.43" >& 2
	exit 1;
fi

if [ "X$TMPDIR" = "X" ] ; then
	echo "environment variable TMPDIR must be defined" >& 2
	exit 1;
fi

#exit with any errors
set -e
set -x

ct=/usr/atria/bin/cleartool
edgsrc=/paraset/src/paraset/CCcc


# create label

VERSION=$2
LABEL=`echo EDG_REL_$VERSION | tr '[a-z]' '[A-Z]'`
cd $edgsrc
$ct mklbtype -nc $LABEL

TEMPDIR=$TMPDIR/tmp.$$
mkdir $TEMPDIR

cd $TEMPDIR

$ct catcs > config_spec.orig
echo 'element * CHECKEDOUT 
element /paraset/src/paraset/CCcc/... /main/edg_releases/LATEST 
element /paraset/src/paraset/CCcc/... /main/LATEST -mkbranch edg_releases
element * /main/LATEST' > config_spec
$ct setcs config_spec

cd $1

addfile()
{
	filename=$1
	dest=$2
	if [ ! -f $dest/$filename ] ; then
		# added file
		if cleartool desc -s $dest | grep CHECKEDOUT; then
			:
		else
			$ct co -c "files added in EDG release $VERSION" $dest
		fi
		cp $filename $dest/$filename
		$ct mkelem -ci -c "file added in EDG release $VERSION" $dest/$filename
	else
		# file wasn't added
		if cmp $filename $dest/$filename; then
			# and wasn't changed
			:
		else
			# file was changed
			$ct co -c "change made in EDG release $VERSION" $dest/$filename
			cp $filename $dest/$filename
			$ct ci -nc $dest/$filename
		fi
	fi
	$ct mklabel -replace $LABEL $dest/$filename
}


# process text files
for x in *.txt; do
	addfile $x $edgsrc/data
done
if cleartool desc -s $edgsrc/data | grep CHECKEDOUT; then
	$ct ci -nc $edgsrc/data
fi
$ct mklabel -replace $LABEL $edgsrc/data


# process include files
for x in *.h; do
	addfile $x $edgsrc/include
done
if cleartool desc -s $edgsrc/include | grep CHECKEDOUT; then
	$ct ci -nc $edgsrc/include
fi
$ct mklabel -replace $LABEL $edgsrc/include


# process src files
for x in *.c; do
	addfile $x $edgsrc/src
done
if cleartool desc -s $edgsrc/src | grep CHECKEDOUT; then
	$ct ci -nc $edgsrc/src
fi
$ct mklabel -replace $LABEL $edgsrc/src


#cleanup
cd $TEMPDIR
$ct setcs config_spec.orig
cd ..
rm -rf $TEMPDIR


# report files that must be merged
# $ct findmerge...

