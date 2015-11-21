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
# script to figure flags for parsers
# 21.Nov.1998 Kit Transue

# The positive_tests and negative_tests directories contain sets of
# source files.  The name of the source
# file--without the extension--is the name of the flag in the EDG parser.
# These files are run through the native compiler.

# For files in positive_tests, the flag should be added to the EDG options
# if the native compiler parses the file without error.

# For files in negative_tests, the flag should be added to the options
# list if the native compiler generates an error on that file.


# Depending on how this is called, select only C or only C++ files:
program_name=`basename $0`
if [ ${program_name} = figure_c_flags.sh ] ; then
	src_ext=c
else
	src_ext=cpp
fi

if [ $# != 0 ]
then
	native_compiler=$*
else
	#native_compiler=cl
	native_compiler="CC -c"
fi

# the initial value of flags that we will be building:
flags=

# confirm that compiler returns status codes:
if $native_compiler good.c > /dev/null 2>&1
then
	dummy=
else
	echo "Compiler returned fail code on good.c!"
	exit 1;
fi

if $native_compiler bad.c > /dev/null 2>&1
then
	echo "Compiler returned OK code on bad.c!"
	exit 1;
fi

for filename in positive_tests/*.${src_ext}; do
	option=`echo $filename | sed 's@.*/\(.*\)\..*@\1@'`;
	echo Testing for option $option
	if $native_compiler $filename > /dev/null 2>&1;
	then
		flags="--$option $flags"
	fi
done

for filename in negative_tests/*.${src_ext}; do
	option=`echo $filename | sed 's@.*/\(.*\)\..*@\1@'`;
	echo Testing for option $option
	if $native_compiler $filename > /dev/null 2>&1;
	then
		dummy_op=
	else
		flags="--$option $flags"
	fi
done

rm *.o
echo $flags
