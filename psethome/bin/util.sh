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

versionUtil="/main/20"
WHICH=$0
first_debug_prefs=yes; export first_debug_prefs
[ "X$DIS_DEBUG_PREFS" != "X" ] && [ "X$first_debug_prefs" = "Xyes" ] && {
   \rm -rf $DIS_DEBUG_PREFS
   first_debug_prefs=no
}
  
PREF_YES="[Yy][Ee][Ss]"
PREF_TRUE="[Tt][Rr][Uu][Ee]"
PREF_NO="[Nn][Oo]"
PREF_FALSE="[Ff][Aa][Ll][Ss][Ee]"


status_OK=0
status_WARNING=2
status_ERROR=3
status_FATAL=4
status_ABORT=5
STATUS=${STATUS:-"$status_OK"}; export STATUS
EXEPID=$$; export EXEPID

actionUtilMessageLog=${actionUtilMessageLog:-""}; export actionUtilMessageLog

funcUtilWARNINGFunc=${funcUtilWARNINGFunc:-"UtilSTATUSNullFunc"}
funcUtilFATALFunc=${funcUtilFATALFunc:-"UtilSTATUSExitFunc"}
funcUtilABORTFunc=${funcUtilABORTFunc:-"UtilSTATUSExitFunc"}

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

PlatformArch()
{

  case `uname -s` in

    HP-UX)
      case `uname -r` in
        ?.09*) echo "hp700" ;;
        ?.10*) echo "hp10" ;;
      esac
      ;;
        
    SunOS)
      case `uname -r` in
        4.*) echo "sun4" ;;
        5.*) echo "sun5" ;;
      esac
      ;;

    IRIX|IRIX64)
      case `uname -r` in
        5.*) echo "irix5" ;;
        6.*) echo "irix6" ;;
      esac
      ;;
 
    Linux)
       echo "linux2"
       ;;

  esac

}


PlatformDependency()

{

  case `PlatformArch` in


    hp700|hp10)
	PATH="$PATH:/etc:/usr/bin/X11"; export PATH
  	AWK=awk; export AWK
	uuencodeExecPath="/usr/bin/uuencode"
	;;

    sun5)
	PATH="$PATH:/usr/sbin/ping:/etc:/usr/openwin/bin:/usr/ccs/bin"; export PATH
        AWK=nawk; export AWK
	uuencodeExecPath="/usr/bin/uuencode"
        ;;
 
    sun4)
	PATH="$PATH:/usr/etc:/etc:/usr/openwin/bin"; export PATH
        AWK=nawk; export AWK
	uuencodeExecPath="/usr/bin/uuencode"
	;;

    irix6)
	PATH="$PATH:/usr/bsd:/usr/etc:/usr/bin/X11:/sbin:/usr/bin"; export PATH
        AWK=nawk; export AWK
	uuencodeExecPath="/usr/bsd/uuencode"
        ;;

    linux2)
	AWK=awk; export AWK
	;;

  esac

}

DIAGNOSTIC()

{

  set -f
  echo ""
  echo "$EXE: DIAGNOSTIC: $*"
  echo "$EXE: $EXE has encountered an unexpected failure. Please contact SET support"
  echo "$EXE: and supply them with the contents of the file $TMPDIR/diagnostic_[csh,sh].$EXEPID"
  echo ""
  set +f

  STATUS=$status_FATAL; export STATUS

  EnvSave sh all > $TMPDIR/diagnostic_sh.$EXEPID
  EnvSave csh all > $TMPDIR/diagnostic_csh.$EXEPID

  Exit 1 

}

ABORT()

{

  set -f
  eval echo "$EXE: ABORT: "`echo $* | sed -e 's%(%\\\(%g' -e 's%)%\\\)%g'` $actionUtilMessageLog
  STATUS=$status_ABORT; export STATUS
  set +f

  # will normally dispatch to a function that exits
  UtilSTATUSCheck

}

FATAL()

{

  set -f
  eval echo "$EXE: FATAL: "`echo $* | sed -e 's%(%\\\(%g' -e 's%)%\\\)%g'` $actionUtilMessageLog
  STATUS=$status_FATAL; export STATUS
  set +f

}

ERROR()

{

  set -f
  eval echo "$EXE: ERROR: "`echo $* | sed -e 's%(%\\\(%g' -e 's%)%\\\)%g'` $actionUtilMessageLog
  if [ "X$STATUS" != "X" ]
  then
    case $STATUS in

      $status_ABORT)
	;;

      $status_FATAL)
	;;

      $status_ERROR)
	;;

      *)
	STATUS=$status_ERROR; export STATUS
	;;

    esac

  fi
  set +f

}

WARNING()

{

  set -f
  eval echo "$EXE: WARNING: "`echo $* | sed -e 's%(%\\\(%g' -e 's%)%\\\)%g'` $actionUtilMessageLog

  if [ "X$STATUS" != "X" ]
  then
    case $STATUS in

      $status_ABORT)
	;;

      $status_FATAL)
	;;

      $status_ERROR)
	;;

      $status_WARNING)
	;;

      *)
	STATUS=$status_WARNING; export STATUS
	;;

    esac
  fi
  set +f

}

LOG()

{

  set -f
  eval echo "$EXE: "`echo $* | sed -e 's%(%\\\(%g' -e 's%)%\\\)%g'` $actionUtilMessageLog
  set +f

}

PlatformDependency

FindFile()
 
{
 
  arguements="$*"
  files=`echo $arguements | $AWK 'BEGIN{RS=":"};{print}'`
  for file in $files
  do
    if [ -f $file ]
    then
      echo $file
      return  0
    fi
  done
  return  1
 
}

GetResourceValue()

{

  resource=$1
  value="+"
  ret=0

  case $resource in

    swap)
	case `PlatformArch` in

	  sun5)
		value=`swap -s | $AWK '{gsub ( "k", "", $11 );print $11}'`
#		ENHANCE error check
		;;

	  sun4)
		value=`pstat -s | $AWK '{gsub ( "k", "", $9 );print $9}'`
#		ENHANCE error check
		;;

	  hp700|hp10)
		if [ -x /etc/swapinfo -t ]
		then
		  value=`swapinfo -t | $AWK '$1 ~ /tot/ {print $4}'`
		else
		  value="+"
		fi
#		ENHANCE error check and correct value for hp700 systems
		;;

	  irix6)
                value=`{ swap -sb | $AWK '{print $(NF-2)}'; echo " / 2"; } | xargs expr`
#               ENHANCE error check
                ;;

	  linux2)
		value= `free -o -k | $AWK ' /Swap:/ { print $2; } ' `
		;;

	esac
	;;

    TMPDIR)
	if [ "X$TMPDIR" != "X" ]
	then
          if [ ! -d "$TMPDIR" ]
	  then
	    ret=1
          else
	    case `PlatformArch` in

	      sun5)
		value=`/bin/df -k $TMPDIR | $AWK '
			BEGIN {
				value="+";
				};
			NR == 1 {
				next;
				}
			NR == 2 && NF == 6 {
				value=$4;
				exit;
				};
			NR == 3 && NF == 5 {
				value=$3;
				exit;
				};
			END {
				print value;
				};
		'`
		;;

	      sun4)
		value=`df $TMPDIR | $AWK '
			BEGIN {
				value="+";
				};
			NR == 1 {
				next;
				}
			NR == 2 && NF == 6 {
				value=$4;
				exit;
				};
			NR == 3 && NF == 5 {
				value=$3;
				exit;
				};
			END {
				print value;
				};
		'`
		;;

	      hp700|hp10)
		value=`bdf $TMPDIR | $AWK '
			BEGIN {
				value="+";
				};
			NR == 1 {
				next;
				}
			NR == 2 && NF == 6 {
				value=$4;
				exit;
				};
			NR == 3 && NF == 5 {
				value=$3;
				exit;
				};
			END {
				print value;
				};
		'`
		;;

		irix6)
                  value=`/usr/bin/df -k $TMPDIR | /usr/bin/tail -1 | $AWK '{print $(NF-2)}'`
                ;;
		
		linux2)
		  value=`/bin/df -k $TMPDIR | $AWK '$0 !~ /Filesystem/ {print $4}'`	
		;;

	    esac
          fi
	else
	  ret=1
	fi
	;;

    memory)
	case `PlatformArch` in

	  sun5)
		value=`prtconf | $AWK '
			BEGIN {
				value="+";
				};
			/Memory size/ {
				value=$3 * 1024;
				};
			END {
				print value;
				};
			'`
		;;

	  sun4)
		value=`dmesg | $AWK '
			BEGIN {
				value="+";
				};
			/mem = .*K/ {
				gsub ( "K", "", $3);
				value=$3;
				};
			END {
				print value;
				};
			'`
		;;

	  hp700|hp10)
		value="+"
		;;

	  irix6)
		value=`{ hinv | grep "Main memory size:" | $AWK '{print $4}'; echo "* 1024"; } | xargs expr`
                ;;

 	  linux2)
		 value=`free -o -k | $AWK '
			/Mem:/ {
			        value=$2;
			      };
			END {
				print value;
				};
			'`
		;;

	esac
	;;

  esac

  echo $value
  return $ret

}

##########
#
# GetPrefValue(prefName)
#
#   Looks in user and system preference files for the
# given preference.
#
##########
GetPrefValue()
{

  prefName=$1

  ### First, check in the user preference file.
  if [ "X$prefsFile" = "X" ]
  then
    prefsFile=`FindFile "$HOME/.psetPrefs:$ADMINDIR/prefs/build.prefs:$PSETHOME/lib/psetPrefs.default"`
    ret=$?
  fi

  if [ $? -ne 0 ]
  then
    FATAL "unable to locate prefs file"
    return 1
  fi

  userValue=`ReadPrefValue $prefName $prefFile`

  if [ "$userValue" = "-" ]
  then
    ### Not in the user's preference file, check the system.
    ReadPrefValue $prefName $PSETHOME/lib/psetPrefs.default
    if [ $? -ne 0 ]
    then
      return 1
    fi
  else
    echo $userValue
  fi

  return $ret
}

##########
#
# ReadPrefValue prefName prefFile
#
#    Read the given preference value from the
# given preference file.
##########
ReadPrefValue()
{
  prefValue=$1
  prefFile=$2

  if [ ! -r $prefsFile ]
  then
    DIAGNOSTIC "unable to open prefs file ($prefsFile)"
    return 1
  fi

  actionPrefValue=`$AWK '
    /^[         ]*#/ {                          # ignore comments
        next;
        };
    $0 ~ /^[    ]*\*/ {
        gsub ( "^[      ]*\\\\*", "" );         # remove leading whitespace before *
        gsub ( "\\\\\\\\", "" );                # need to remove line extending char....
	if ( match ( $0, ":" ) != 0 ) {
	  # if we found a :
	  prefColonPosition=match ( $0, ":" );
	  prefValueLength=length($0) - prefColonPosition;
          prefCrntValue = substr ( $0, prefColonPosition+1 , prefValueLength );
          prefCrntName = substr ( $0, 0, prefColonPosition -1 );
	  if ( prefCrntName ~ prefName ) {
            prefValue[prefCrntName] = prefCrntValue;
	  }
	}
        next;
        };
    /^[         ]*$/ {				# remove whitespace lines
        next;
        };
        {
        gsub ( "\\\\\\\\", "" );                # need to remove line extending char....
	gsub ( "[ 	]*", "" );
        prefValue[prefCrntName]=prefValue[prefCrntName]" "$0;
        };
    END {
        for ( prefCrntName in prefValue ) {
	  if ( "X"prefCrntName !~ /^X$/ ) {
            if ( prefName ~ prefCrntName ) {
              if ( prefValue[prefCrntName] ~ /^[         ]$/ ) {
                print "echo +";
                exit ( 1 );
              } else {
                print "echo ", prefValue[prefCrntName] ;
                exit ( 0 );
              };
            };
	  };
	};
        print "echo -";
        exit ( 2 );
        };
  ' prefName=$prefName $prefsFile`
  ret=$?

  prefValue=`eval $actionPrefValue`
  if [ "X$prefValue" = "X" ]
  then
    prefValue="+"		# there was probably an environment variable in the
				# preference and it was not set
    ret=1			# by definition incomplete preference value
  fi

  [ "X$DIS_DEBUG_PREFS" != "X" ] && {
     echo "prefFile=$2, $1= $prefValue" >> $DIS_DEBUG_PREFS
  }

  echo $prefValue

  return $ret

}


CheckHost()

{

  hostName=$1
  userName=`whoami`

  case `PlatformArch` in

    sun*)
		ping $hostName > /dev/null 2>&1; ret=$?
		if [ $ret -ne 0 ]
		then
		  ERROR "Host $hostName is not available on net"
		  return $ret
		fi
		;;

    hp700|hp10)
		ping $hostName 64 10 | $AWK '
		  BEGIN {
			ret=1;
			};
		  /bytes from/ {
			ret=0;
			exit ( ret );
			}
		  END {
			exit ( ret );
			};
		'
		ret=$?
		if [ $ret -ne 0 ]
		then
		  ERROR "Host $hostName is not available on net"
		  return $ret
		fi
		;;

    linux2)
		ping $hostName -c 10 | $AWK '
		   BEGIN {
			ret=1;
			};
		   /bytes from/ {
			ret=0;
			exit ( ret );
			}
		   END {
			exit ( ret );
			};
		   '
		   ret=$?
		   if [ $ret -ne 0 ]
		   then
		     ERROR "Host $hostName is not available on net"
		     return $ret
		   fi
		   ;;

  esac

  ruserName=`rsh $hostName whoami 2> /dev/null`
  ret=$?
  if [ $ret -ne 0 ]
  then
    ERROR "Host $hostName does not accept login by $userName"
    return 1
  fi

  return 0

}

EnvSet()

{

  matchPattern=$1
  shift

  envSetVariablesList="$*"

  if [ $# -lt 1 ]
  then
    DIAGNOSTIC "EnvSet called with to few paramaters"
  fi

  for envVariableEntry in $envSetVariablesList
  do

    envVariable=`echo $envVariableEntry|$AWK -F":" '{sub ( "=.*$", "", $1 ); print $1}'`
    envValue=`echo $envVariableEntry|$AWK -F":" '{sub ( "^.*=", "", $1 ); print $1}'`
    envPattern=`echo $envVariableEntry|$AWK -F":" '{if (NF > 1 ) {print $2} else {print "+" };}'`

    case $envPattern in

	+)
		eval "$envVariable=$envValue; export $envVariable"
		;;

	$matchPattern)
		eval "$envVariable=$envValue; export $envVariable"
		;;

	*)
		;;

    esac

  done

}

EnvSave()

{

  shellType=$1
  shift
  variableList="$@"

  arguements="$shellType $variableList"

  $AWK '
    BEGIN {
	FS="=";
	argc=1;
	while ( "env" | getline > 0 ) {
	  #
	  # variable[variable name]=variable value
	  #
	  variable[$1]=$2;
	};
	if ( ARGV[argc] == "csh" || ARGV[argc] == "sh" || ARGV[argc] == "ksh" ){
	  shell=ARGV[argc++];
	}
	if ( ARGV[argc] == "all" ) {
	  #
	  # we want all enviroment variables
	  #
	  for ( crntVar in variable ) {
	    if ( variable[crntVar] ) {
	      if ( shell == "csh" ) {
	        printf ( "setenv %s \"%s\"\n", crntVar, variable[crntVar], crntVar);
	      } else if ( shell == "sh" || shell == "ksh" ) {
	        printf ( "%s=\"%s\";export %s\n",crntVar,variable[crntVar],crntVar);
	      };
	    };
	  };
	} else {
	  #
	  # we want a list of enviroment variables
	  #
	  for ( ; argc < ARGC; argc++ ) {
	    if ( variable[ARGV[argc]] ) {
	      #
	      # if the variable was found earlier
	      #
	      if ( shell == "csh" ) {
	        printf ( "setenv %s \"%s\"\n", ARGV[argc], variable[ARGV[argc]], ARGV[argc] );
	      } else if ( shell == "sh" || shell == "ksh" ) {
	        printf ( "%s=\"%s\";export %s\n", ARGV[argc], variable[ARGV[argc]], ARGV[argc] );
	      };
	    };
	  };
	};
	};
  ' $arguements

}

NormalizePath()

{

  path=$1

  expr $path : '^/' > /dev/null
  if [ $? -eq 0 ]
  then
    echo $path
  else
    echo `pwd`/$path | $AWK '
	BEGIN {
		FS="/";
		};
	$2 ~ /tmp_mnt/ && $3 ~ /net/ {
		gsub ( "/tmp_mnt", "" );
		};
		{
		print;
		};
	'
  fi

}

Xmessage()
 
{
 
  message=$1
 
  xterm -geometry 80x5+200+200 -e xmessage $message
 
}

ProcExists()

{

  set | grep "$*\(\)" > /dev/null 2>&1

}

Exit()

{

  if ProcExists Cleanup; then
  
    Cleanup

  fi

  exit $*

}

UtilSTATUSExitFunc()

{

  FATAL ""
  FATAL "*** A FATAL error has occurred that prevents $EXE from continuing ***"
  FATAL ""

  UtilMessageLog FLUSH
  Exit $STATUS

}

UtilSTATUSNullFunc()

{

  UtilMessageLog FLUSH
  return $STATUS

}

UtilSTATUSCheck()

{

  case "X$STATUS" in

    X)
	DIAGNOSTIC "UtilSTATUSCheck: STATUS is unset"
	;;

    X$status_ABORT)
	eval $funcUtilABORTFunc;
	;;

    X$status_FATAL)
	eval $funcUtilFATALFunc;
	;;

    X$status_WARNING)
	if [ "X$funcUtilWARNINGFunc" != "X" ]
	then
	  eval $funcUtilWARNINGFunc; STATUS=$?; export STATUS
	fi
	;;

    X*)
	;;

  esac

  return $STATUS

}

UtilMessageLog()

{

  if [ $# -ne 1 ]
  then
    DIAGNOSTIC "UtilMessageLog: called with incorrect params ($*)"
  fi
  mode=$1
  case $mode in

    INITIALIZE)
	tmpMessageLogFile="/tmp/$EXE.$EXEPID"; export tmpMessageLogFile
					# used for initial logging of util.sh messages
	actionUtilMessageLog=">> $tmpMessageLogFile 2>&1"; export actionUtilMessageLog
					# used for initial logging of util.sh messages, NB
					# must use output *APPEND* or you only get last message
	;;

    FLUSH)
	#
	# We want to collect all the error output and display it in one place at
	# one time. This ensures that it will be placed in the log file since by this
	# actionLog will have been initialized or we would have exited badly
	# We need to reset actionUtilMessageLog so that output will come 
	# immediately now. Also , remove the tmp file
	# We need to be able to flush this anytime we exit prematurely
	#
	if [ -f $tmpMessageLogFile ]
	then
	  eval cat $tmpMessageLogFile
	  rm -f $tmpMessageLogFile
	fi
	;;

    DEINITIALIZE)
	#
	# We no longer want to cache error output so reset action variable
	#
	actionUtilMessageLog=""; export actionUtilMessageLog
	;;

    *)
	DIAGNOSTIC "UtilMessageLog: invalid mode ($mode)"
	;;

  esac
  
}

Value()

{

  if [ $# -ne 2 ]; then
    FATAL "Value: Invalid parameters. Please contact THECOMPANY support."
  fi

  VAR="$1"
  DEFAULT="$2"

  VAL="`GetPrefValue psetPrefs.$VAR`"
  case "$VAL" in

    -|+)
      VAL="$DEFAULT"
      ;;

    *)
      ;;

  esac

  eval $VAR='"$VAL"'

}

ValueAbort()

{

  if [ $# -ne 1 ]; then
    FATAL "ValueAbort: Invalid parameters. Please contact THECOMPANY support."
  fi

  VAR="$1"

  VAL="`GetPrefValue psetPrefs.$VAR`"
  case "$VAL" in

    -)
      FATAL "Preference value (psetPrefs.$VAR) is missing"
      ;;

    +)
      FATAL "Preference value (psetPrefs.$VAR) is incomplete"
      ;;

    *)
      ;;

  esac

  eval $VAR='"$VAL"'

}

ValueBinary()

{

  if [ $# -ne 2 -a $# -ne 3 ]; then
    FATAL "ValueBinary: Invalid parameters. Please contact THECOMPANY support."
  fi

  VAR="$1"
  DEFAULT="$2"
  PREF="psetPrefs.${3:-$VAR}"
  
  VAL="`GetPrefValue $PREF`"

  case "$VAL" in

    1|2|$PREF_YES|$PREF_TRUE)
      VAL="YES"
      ;;

    0|-1|$PREF_NO|$PREF_FALSE)
      VAL="NO"
      ;;

    *)
      VAL="$DEFAULT"
      ;;

  esac

  eval $VAR='"$VAL"'

}
