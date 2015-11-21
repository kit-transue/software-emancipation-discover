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

versionModelUtil="/main/2"
WHICH=$0
funcUtilFATALFunc="ModelutilExitFunc"; export funcUtilFATALFunc
funcUtilABORTFunc="ModelutilExitFunc"; export funcUtilABORTFunc

GetUserOptions()
{
  while [ $# -gt 0 ]
  do
    case $1 in

      -which)
		echo $WHICH
		;;

      *)

    esac
    shift
  done

}

GetUserOptions $*

ModelutilExitFunc()
{

  FATAL ""
  FATAL "      *** A SITE SPECIFIC SETUP PROBLEM HAS OCCURRED ***"
  FATAL "*** PLEASE CONTACT YOUR CODEINTEGRITY ADMINISTRATOR FOR ASSISTANCE ***"
  FATAL ""

  UtilMessageLog FLUSH

  Exit $STATUS

}

ModelutilNBFunc()

{

  WARNING ""
  WARNING " *** Non fatal problems have occurred, proceeding ***"
  WARNING "*** Please  make your DIScover administrator aware ***"
  WARNING ""

  UtilMessageLog FLUSH

  return $STATUS

}

projectFile2List ()

{
	echo `cat $projectListFile`
}


ProjectList2HostList()
{
  projectList=`projectFile2List`
  res=`projectFile2List | tr ' ' '\012' | $AWK '{print $2}' FS=":" | sort -u `

  if [ "X$res" = "X" ]; then
	echo $hostAlias
  else	
        echo $res | tr ' ' '\012' 
  fi
}

ProjectList2HostList1()

{
  projectList=`projectFile2List`
  echo "$projectList" | $AWK '
    function ProjEntry2Host( projEntry,           entryLen, hostName ) {
      entryLen=split ( projEntry, entryArray, ":" );
      if ( entryLen == 2 ) {
        if ( entryArray[2] !~ /^$/ ) {
          hostName=entryArray[2];
        } else {
          hostName=localHost;
        }
      } else {
        hostName=localHost;
      }
      return ( hostName );
    }

    {
	projectList=$0;
	};

    END {
        projectListLen=split ( projectList, projectListArray );
        for ( projectListIdx in projectListArray ) {
          projHost=ProjEntry2Host(projectListArray[projectListIdx]);
	  projHostArray[projHost]=projHost;
        };
	for ( projHost in projHostArray ) {
	  print projHost
	}
	};
  ' localHost=$hostAlias

}

ProjectList2ProjNameList()
{
  projectList=`projectFile2List`

# echo "`date`: hostAlias=$hostAlias; projHost=$projHost" >> /usr/tmp/hosts_debug

  for el in `projectFile2List | tr ' ' '\012'`
  do
      res=`echo $el | sed -n "s@^[	 ]*/*\(.*\):$projHost@\1@p"`

      if [ "X$res" = "X" ]; then
	  hst=`echo $el | sed -n "s@^[	 ]*/*.*:\(.*\)@\1@p"`
	  if [ "X$hst" = "X" -a "X$hostAlias" = "X$projHost" ]; then
	  	echo $el | sed -n 's@^[	 ]*/*\(.*\)@\1@p'

#		echo -n "    df host "                    >> /usr/tmp/hosts_debug
#	  	echo $el | sed -n 's@^[	 ]*/*\(.*\)@\1@p' >> /usr/tmp/hosts_debug
	  fi
      else	
  	  echo $res 

# 	  echo -n "    by host "  >> /usr/tmp/hosts_debug
#  	  echo $res               >> /usr/tmp/hosts_debug

      fi
  done
}

ProjectList2ProjNameList1()
{

  #
  # ENHANCEMENT
  # NB there is duplication between this awk script and that in ProjectList2HostList()
  # this is bad programming and should be fixed
  #
   projectList=`projectFile2List`
   echo "$projectList" | $AWK '
    function ProjEntry2Name( projEntry,           entryLen, projName ) {
       entryLen=split ( projEntry, entryArray, ":" );
       projName=entryArray[1];
       gsub ( "^[ 	]*/", "", projName );
       return ( projName );

    };
    function ProjEntry2Host( projEntry,           entryLen, hostName ) {
      entryLen=split ( projEntry, entryArray, ":" );
      if ( entryLen == 2 ) {
        if ( entryArray[2] !~ /^$/ ) {
          hostName=entryArray[2];
        } else {
          hostName=localHost;
        }
      } else {
        hostName=localHost;
      }

      return ( hostName );

    };
    {
	projectList=$0;
	};

    END {
        projectListLen=split ( projectList, projectListArray );
        for ( projectListIdx in projectListArray ) {
          listProjHost=ProjEntry2Host(projectListArray[projectListIdx]);
          projName=ProjEntry2Name(projectListArray[projectListIdx]);
	  if ( listProjHost == projHost ) {
            printf ( "%s\n", projName );
	  };
        };
        };

  ' projHost=$projHost localHost=$hostAlias

}
