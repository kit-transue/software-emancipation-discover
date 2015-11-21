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

versionUtil="0.0.xx"

PREF_YES="[Yy][Ee][Ss]"
PREF_TRUE="[Tt][Rr][Uu][Ee]"
PREF_NO="[Nn][Oo]"
PREF_FALSE="[Ff][Aa][Ll][Ss][Ee]"

PlatformArch()
{

  case `uname -r` in

    A.0*)
	echo "hp700"
	;;

    4.*)
	echo "sun4"
	;;

    5.*)
	echo "sun5"
	;;
 
  esac

}


PlatformDependency()

{

  case `PlatformArch` in


    hp700)
  	AWK=awk; export AWK
	;;

    sun5)
	PATH="$PATH:/usr/sbin/ping"; export PATH
        AWK=nawk; export AWK
        ;;
 
    sun4)
	PATH="$PATH:/usr/etc"; export PATH
        AWK=nawk; export AWK
	;;

  esac

}

DIAGNOSTIC()

{

  echo ""
  echo "$EXE: DIAGNOSTIC: $*"
  echo "$EXE: $EXE has encountered an unexpected failure. Please contact SET support"
  echo "$EXE: and supply them with the contents of the file $TMPDIR/envsave"
  echo ""

  STATUS=FATAL; export STATUS

  EnvSave > $TMPDIR/envsave

  exit 1 

}

FATAL()

{

  echo "$EXE: FATAL: $*"
  STATUS=FATAL; export STATUS

}

ERROR()

{

  echo "$EXE: ERROR: $*"
  if [ "X$STATUS" != "X" ]
  then
    case $STATUS in

      FATAL)
	;;

      ERROR)
	;;

      *)
	STATUS=ERROR; export STATUS
	;;

    esac

  fi

}

WARNING()

{

  echo "$EXE: WARNING: $*"
  if [ "X$STATUS" != "X" ]
  then
    case $STATUS in

      FATAL)
	;;

      ERROR)
	;;

      WARNING)
	;;

      *)
	STATUS=WARNING; export STATUS
	;;

    esac

  fi

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
  value=-1
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

	  hp700)
		if [ -x /etc/swapinfo -t ]
		then
		  value=`swapinfo -t | $AWK '$1 ~ /tot/ {print $4}'`
		else
		  value="+"
		fi
#		ENHANCE error check and correct value for hp700 systems
		;;

	esac
	;;

    TMPDIR)
	if [ "X$TMPDIR" != "X" ]
	then
	  case `PlatformArch` in

	    sun5)
		value=`/bin/df -k $TMPDIR | $AWK '$0 !~ /Filesystem/ {print $4}'`
#		ENHANCE error check
		;;

	    sun4)
		value=`df $TMPDIR | $AWK '$0 !~ /Filesystem/ {print $4}'`
#		ENHANCE error check
		;;

	    hp700)
		value=`bdf $TMPDIR | $AWK '$0 !~ /Filesystem/ {print $4}'`
#		ENHANCE error check
		;;

	  esac
	else
	  ret=1
	fi
	;;

    memory)
	case `PlatformArch` in

	  sun5)
		value=`dmesg | $AWK '
			/mem = .*K/ {
				gsub ( "K", "", $3);
				value=$3;
				};
			END {
				print value;
				};
			'`
		;;

	  sun4)
		value=`dmesg | $AWK '
			/mem = .*K/ {
				gsub ( "K", "", $3);
				value=$3;
				};
			END {
				print value;
				};
			'`
		;;

	  hp700)
		value="+"
		;;

	esac
	;;

  esac

  echo $value
  return $ret

}

GetPrefValue()

{

  prefName=$1

  if [ "X$prefsFile" = "X" ]
  then
    prefsFile=`FindFile "$HOME/.psetPrefs:$PSETHOME/lib/psetPrefs.default"`
    ret=$?
  fi

  if [ $? -ne 0 ]
  then
    FATAL "unable to locate prefs file"
    return 1
  fi
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
        prefCrntValue = substr ( $0, match ( $0, ":" ) +1 );
        prefCrntName = substr ( $0, 0, match ( $0, ":" ) -1 );
        prefValue[prefCrntName] = prefCrntValue;
        next;
        };
    /^[         ]*$/ {
        next;
        };
        {
        gsub ( "\\\\\\\\", "" );                # need to remove line extending char....
        prefValue[prefCrntName]=prefValue[prefCrntName]" "$0;
        };
    END {
        for ( prefCrntValue in prefValue ) {
          if ( prefName ~ prefCrntValue ) {
            if ( prefValue[prefCrntValue] ~ /^[         ]$/ ) {
              print "echo +";
              exit ( 1 );
            } else {
              print "echo ", prefValue[prefCrntValue] ;
              exit ( 0 );
            };
          };
        }
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

    hp700)
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

  shellType=sh
  variableList=all

  arguements="$shellType $variableList"

  $AWK '
    BEGIN {
	FS="=";
	argc=1;
	while ( "env" | getline > 0 ) {
	  variable[$1]=$2;
	};
	if ( ARGV[argc] == "csh" || ARGV[argc] == "sh" || ARGV[argc] == "ksh" ){
	  shell=ARGV[argc++];
	}
	if ( ARGV[argc] == "all" ) {
	  for ( crntVar in variable ) {
	    if ( variable[crntVar] ) {
	      if ( shell == "csh" ) {
	        printf ( "setenv %s %s\n", crntVar, variable[crntVar], crntVar);
	      } else if ( shell == "sh" || shell == "ksh" ) {
	        printf ( "%s=\"%s\";export %s\n",crntVar,variable[crntVar],crntVar);
	      };
	    };
	  };
	} else {
	  for ( ; argc < ARGC; argc++ ) {
	    if ( variable[ARGV[argc]] ) {
	      if ( shell == "csh" ) {
	        printf ( "setenv %s \"%s\"\n", ARGV[argc], variable[ARGV[argc]], ARGV[argc] );
	      } else if ( shell == "sh" || shell == "ksh" ) {
	        printf ( "%s=%s;export %s\n", ARGV[argc], variable[ARGV[argc]], ARGV[argc] );
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
    echo `pwd`/$path
  fi

}

Xmessage()
 
{
 
  message=$1
 
  xterm -geometry 80x5+200+200 -e xmessage $message
 
}

