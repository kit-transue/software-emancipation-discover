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

# 5.Oct.1999 Kit Transue
# start in same directory as .book and .fm files
# Developed with FrameMaker 5.5.6.
# Relies on naming convention .fm and .book.

usage() {
	echo 'Usage: fmbatch_print_book.sh [-help] [-save location] [-diag]' 1>&2
	echo 'With no arguments, updates and prints-to-pdf book file in current directory' 1>&2
	echo '-diag does not call fmbatch, but sends batch commands to stdout' 1>&2
	echo '-save option, updates and saves a current copy to <location>' 1>&2
	exit 1
}

# what to do with commands when we're done:
ACTION=fmbatch

MODE=print
while [ "$#" -ne 0 ]
do
	case "$1" in
		-save)
			shift
			if [ "$#" -lt 1 ]
			then
				usage
				exit 1
			fi
			SAVEROOT="$1"
			shift
			;;
		-diag)
			ACTION=cat
			shift
			;;
		-help)
			usage
			exit 0
			;;
		*)
			usage
			exit 1
			;;
	esac
done

TMPFILE=/usr/tmp/fm_to_pdf.tmp.$$

# file containing global variable definitions such as product names:
VARDEFNS=/DOCUMENTATION/external/enduser/development/common/DiscoverVariables.fm
# file containing book-specific definitions, such as title or conditional text settings:
LOCALDEFNS=bookVariables.fm
# create a mif file containing variables for build and product version
MIFDATE=/usr/tmp/version.tmp.$$.mif
/paraset/tools/builder/create_mif_timestamp.sh > "${MIFDATE}"

BOOKFILE=`ls *.book | grep -v '\.backup\.' | grep -v '\.recover\.' | grep -v '\.auto\.' | head -1`
FMFILES=`ls *.fm *.mif | grep -v '\.backup\.' | grep -v '\.recover\.' | grep -v '\.auto\.' | grep -v 'bookVariables.fm'`
GENFILES=`ls *TOC.fm *IX.fm | grep -v '\.backup\.' | grep -v '\.recover\.' | grep -v '\.auto\.'`

# prepare list of commands to open files:
for x in "${BOOKFILE}" ${FMFILES}; do
	echo "Open ${x}" >> "${TMPFILE}"
done

echo "Open ${VARDEFNS}" >> "${TMPFILE}"
echo "Open ${MIFDATE}" >> "${TMPFILE}"
if [ -r "${LOCALDEFNS}" ]; then
	echo "Open ${LOCALDEFNS}" >> "${TMPFILE}"
fi
# import global and book-specific variables:
# may want to use MIF to automatically maintain version number and date.
for x in $FMFILES; do
	echo "ImportFormats v ${x} ${VARDEFNS}" >> "${TMPFILE}"
	echo "ImportFormats v ${x} ${MIFDATE}" >> "${TMPFILE}"
	if [ -r "${LOCALDEFNS}" ]; then
		# set local variables:
		echo "ImportFormats v ${x} ${LOCALDEFNS}" >> "${TMPFILE}"
		# set conditional text:
		#echo "ImportFormats x ${x} ${LOCALDEFNS}" >> "${TMPFILE}"
	fi
done

echo "Update ${BOOKFILE}" >> "${TMPFILE}"
case "${MODE}" in
	print)
		echo "Print ${BOOKFILE} ${VARDEFNS}" >> "${TMPFILE}"
		;;
	save)
		for x in ${FMFILES}; do
			echo "SaveAs d ${x} ${SAVEROOT}/${x}" >> "${TMPFILE}"
		done
		;;
esac


cat "${TMPFILE}" | "${ACTION}"

# don't remove .mif for diagnostic purposes:
if [ "${ACTION}" = "fmbatch" ]
then
	rm "${TMPFILE}"
fi
