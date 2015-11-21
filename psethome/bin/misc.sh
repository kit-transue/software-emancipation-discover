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

funct=$1
shift
args=$*

. `$PSETHOME/bin/util.sh -which`
. `$PSETHOME/bin/modelutil.sh -which`

env_save()
{
  sh_file=$1
  csh_file=$2
  EnvSave sh all > $sh_file	
  EnvSave csh all > $csh_file
}

set_debug_vars()
{
  EnvSet "$args"
}

log_pref_values()
{
  EXE="start_discover"
  echo "$EXE: Preference File Values:"
  $AWK '
  /^[ 	]*$/ {
        next;
        };
  /^#/	{
	next;
	};
	{
	gsub ( "^[              ]*\\\\*", "" );
	gsub ( "^\\*psetPrefs\.", "" );
	prefCrntValue = substr ( $0, match ( $0, ":" ) +1 );
	gsub ( "^[ 	]*", "", prefCrntValue );
	prefCrntName = substr ( $0, 0, match ( $0, ":" ) -1 );
	if ( prefCrntName ~ /^$/ ) {
	  separator=" ";
	} else {
	  separator="=";
	};
	printf ( "%s:   %-35.35s %s %s\n", EXE, prefCrntName, separator, prefCrntValue );
	};
  ' EXE=$EXE $prefsFile 

}

eval $funct $args
