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
# Install bugclient and bugcmd in appropriate directories
# Steve Schmidt
#

usage() {
	echo "$0: Install bugclient and bugcmd in appropriate directories"
	echo ""
	echo "Usage::"
	echo "     $0 <path>"
	echo "         Delete the existing bugtool and bugcmd for each"
	echo "         architecture and copy the new bugtool and bugcmd"
	echo "         from <path>/$Arch to the appropriate area."
	echo ""
	exit
}

# Copyfile will take three arguments -
CopyFile() {
	if [ -f $1 ]
	then
		if [ -f $2 ]
		then
		   rm -f $2.OLD
		   mv $2 $2.OLD
		fi
		cp $1 $2
		chmod 755 $2
	else
		echo "ERROR, file $1 not found"
		exit 1
	fi
}

#
# begin main
#

arches="sun5 sun4 irix6 hp700"
targetbase="/net/oxen/vol04/share"

if [ ! -d "$1" ] ; then
	echo "ERROR, invalid arguments"
	usage
	exit
else
	source="$1"
fi

CopyFile $source/legal_subsystems $targetbase/bin/.bugtool/legal_subsystems
CopyFile $source/bugtool $targetbase/bin/bugtool

for name in $arches
do
	CopyFile $source/$name/bugclient $targetbase/$name/bin/bugclient
	CopyFile $source/$name/bugcmd $targetbase/$name/bin/bugcmd
done
