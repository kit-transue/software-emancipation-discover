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
# /paraset/tools/builder/customer_release.sh
#

# 961023 kit transue:  used to read <quote>
#  uses /paraset/tools/builder/customer_exclude
# <endquote>, but I can't find this file.

# Make a customer installation area

# NOTE: This has been tested on sun4.  There may be incomatibilty
#       problems on other Unix versions, esp. with date formats


SUPPORTBASE=/net/electra/local1b/nexus/Releases
#SUPPORTVER is set later, using version discover reports
SUPPORTDIR=discover

TIMEOUT=15; export timeout

USAGE1='Usage: customer_release.sh -smart [-version <ver-num> | -stable ] { hp700 | sun4 | sun5 | irix6 }'
USAGE2='       customer_release.sh <from-dir> <to-dir>'


here=`pwd`



#----------------------------------
# First check for a 'smart' copy.  The other parameter is the
# platform: hp700, sun4 or sun5.  The optional param of -stable
# will copy from stable on electra.  If we say -version then
# the next parameter will be the version such as 3.1.167.
# Otherwise we copy from the  respective build/install areas 
# with the lastest version.

if [ "$1" = "-smart" ] ; then
  shift

  if [ "$1" = "-stable" ] ; then
    stable=1
    echo 'Sorry, -stable option not supported yet'
    exit 1
    shift
 
  elif  [ "$1" = "-version" ] ; then
    shift
    version="$1"
    shift
  fi

  # Finally figure out where to copy from
  case "$1" in
    sun4 | sun5 | hp700 | irix6)  Arch=$1; export Arch ;;
    * )     echo $USAGE1 ; echo $USAGE2
       exit 1 ;;
  esac

  # Depend on the program 'latest' to give us mostly what we want
  # It must return the -O or -g directory name under the 
  # latest version.  Change to any user specified version then
  # make sure we are in the -O directory
  FROM=`latest ${Arch}`
  FROM=`dirname ${FROM}`

  if [ "${version}" ] ; then
    FROM="`dirname ${FROM}`/${version}"
  fi

  FROM=${FROM}/${Arch}-O

 
  # Construct the version name based on what the script thinks this is:
  # the shell script runs independent of architecture, so this call is OK
  SUPPORTVER=`$FROM/bin/discover -v | grep DISCOVER | awk '{ print $4 }' `

  # Construct the directory name based on the date of
  # aset
  if [ -f $FROM/bin/aset ] ; then
    ASETM=`ls -l ${FROM}/bin/aset | awk '{ print $5 }' `
    ASETD=`ls -l ${FROM}/bin/aset | awk '{ print $6 }' `
    ASETY=`ls -l ${FROM}/bin/aset | awk '{ print $7 }' `
  else
    echo "Could not find ${FROM}/bin/aset"
    exit 1
  fi

  ASETM=`echo ${ASETM} | awk '{ printf "%.2d", (index("JanFebMarAprMayJunJulAugSepOctNovDec", $1) + 2) / 3 }'`



  if echo $ASETY | grep ':' >/dev/null ; then
    ASETY=`date +%y`
    if [ `date +%h | awk '{ printf "%.2d", (index("JanFebMarAprMayJunJulAugSepOctNovDec", $1) + 2) / 3 }'` -lt $ASETM ] ; then
      ASETY=`expr $ASETY - 1`
    fi

  else
    ASETY=`echo ${ASETY} | awk '{ print substr($0,3,2) }'`
  fi
  
  if [ $ASETD -lt 10 ] ; then
    ASETD=0${ASETD}
  fi

  ASETDATE=${ASETY}${ASETM}${ASETD}


  # Now set up our TO direcotry
  if [ "${Arch}" = "hp700" ] ;then
    # Special case until support renames their area to hp700
    TO=${SUPPORTBASE}/hp/${SUPPORTVER}
  else
    TO=${SUPPORTBASE}/${Arch}/${SUPPORTVER}
  fi    
  
  TO=${TO}/${ASETDATE}/${SUPPORTDIR}


  # Now besure we have constructed valid directory paths
  if [ \! -d "$FROM" ] ; then
    echo "$FROM is not a valid directory"
    exit 1
  fi

  if [ \! -d "$TO" ] ; then
    if mkdir -p $TO ; then
      :
    else
      echo "$TO is not a valid directory"
      exit 1
    fi
  fi


elif [ $# -ne 2 ] || [ \! -d $1 ] || [ \! -d $2 ] ; then
  #----------------------------------
  # Not 'smart' copy so try

  echo $USAGE1 ; echo $USAGE2
  exit 1

else
  #----------------------------------
  # The user wants to copy explicitly
  FROM=$1
  TO=$2
  [ "$FROM" = "." ] && FROM=$here
  [ "$TO" = "." ] && TO=$here

fi

export FROM TO


if [ \! -w $TO ] ; then
  echo
  echo "Directory ${TO} not writable. Exiting"
  echo
  exit 1
fi

echo
 
# Check that TO dircectory is empty.  If not warn user
# Note we should have already checked that $TO is a
# directory

cd $TO
if [ `ls | wc -l` -gt 0 ] ; then
  toempty=1
  echo
  echo "WARNING: TO is not empty: $TO"
  echo
  echo "         You should ^C / kill this command and explicitly"
  echo "         remove all files and directories"
  echo
fi
cd $here		# reset so others don't complain





#----------------------------------
# OK. Ready to begin but warn user first

echo
echo "    Installing "
echo "        from: $FROM "
echo "        into: $TO"
echo
echo
echo "    You have $TIMEOUT seconds before copy begins..."
echo


sleep $TIMEOUT

echo -n "    STARTED at "
date


#----------------------------------
# tar over the files
#
cd $FROM
i=0
# 961023 kit transue: bin/debug and aset.* changes made 960522? in builder/private view.
gtar -cf - --exclude=aset.dir --exclude aset.pag \
	--exclude='*pure*' --exclude=bin/debug --exclude='aset\.*' \
	. |   (cd $TO ; gtar -xBvpf - ) | \
	while (read a) ; do
	  i=`expr $i + 1`
	  if [ $i -gt 10 ] ;then
	    echo -n '.'
	    i=0
	  fi
	done


#----------------------------------
# Do some post-copy checking and cleanup
#

if [  ! -f  $TO/bin/aset ] ; then
	echo " *"
	echo " * ERROR: aset not found "
	echo " * Exiting.."
	exit 1
fi




#----------------------------------
#  Strip the labels from the executables.
#  This should be done using the host build 
#  machine remotely to do the stripping, but
#  is not done that way;  this script should
#  be run from a machine of the appropriate
#  architecture.  sschmidt 10/7/98
#

if [ -x /usr/ccs/bin/strip ]; then
  strip=/usr/ccs/bin/strip  #sun5 and sun4
else
  strip=/bin/strip #hp700 and irix6
fi


installation=`echo $FROM | awk -F/ '{print $NF}'`

echo
echo  "     Stripping executables"
$strip $TO/bin/* > /dev/null 2>&1
$strip $TO/bin/package/* > /dev/null 2>&1
$strip $TO/bin/tests/* > /dev/null 2>&1
$strip $TO/bin/generic/* > /dev/null 2>&1

echo
echo -n  "     Installation finished at "
date

exit

#------------------------------
# Don't bother with this anymore
#




chown -R 0 $TO
if [ $? -ne 0 ] ; then
	echo 
	echo "     WARNING: Could not set ownership to root."	
	echo "              Please do this by hand."
fi
chgrp -R 1 $TO
if [ $? -ne 0 ] ; then
	echo 
	echo "     WARNING: Could not set group to root."	
	echo "              Please do this by hand." 
fi
cd $here
exit 0



