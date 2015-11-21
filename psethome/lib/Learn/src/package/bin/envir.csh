#!/bin/csh

#
# set the root of the admin directory
#
set root = "$1"

if ( $root == "" ) then
	set root=`pwd`
endif

#
# set the .pdf variables
#
setenv ADMINDIR $HOME/DISCOVER/Learn/AdminDir
echo "AdminDir at" $ADMINDIR 

set ssroot = $ADMINDIR/../src
set psroot = $ADMINDIR/../private
set smroot = $ADMINDIR/model/SHARED
set pmroot = $ADMINDIR/model/private
set sessrt = $ADMINDIR/../src/package/Sessions

setenv admin		$ADMINDIR
setenv sharedSrcRoot    $ssroot
setenv privateSrcRoot   $psroot
setenv sharedModelRoot  $smroot
setenv privateModelRoot $pmroot
setenv sessions         $sessrt
setenv NO_OPT_CHECK     1

#
# warn if the directories do not exist
#
if ( ! (-d $sharedSrcRoot) ) then
	echo "  no shared source directory" $ssroot
endif
if ( ! (-d $privateSrcRoot) ) then
	echo "  no private source directory" $psroot
endif
if ( ! (-d $sharedModelRoot) ) then
	echo "  no shared model directory" $smroot
endif
if ( ! (-d $privateModelRoot) ) then
	echo "  no private model directory" $pmroot
endif
if ( -d $sessions ) then
	echo "  [sessions] set to" $sessions
else
	echo "  no directory for PACKAGE/sw sessions" $sessrt
endif
