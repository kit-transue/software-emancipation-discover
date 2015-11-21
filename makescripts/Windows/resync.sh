#!/bin/ksh

#
# build all relevant executables
#

EXELIST=`cat ./win_projects.lst` 

MAKEMSG='echo'

#
# build executables
#

BUILDROOT=/si/Projects

BUILDTIME=`date '+%a %b %e %T %Z %Y'`
$MAKEMSG "Resynchronizing started $BUILDTIME..."
for exei in $EXELIST
do
  BUILDDIR=${exei#*:} # trying to get directory name 
                      # it should be separated by ':' from project name
  if [ -z ${BUILDDIR} ] ; then # if directory name is not specified - assume that
                               # a project name and a directory name are the same
     BUILDDIR=${exei}
  fi
  exei=${exei%%:*} # get a project name
  projname=${BUILDROOT}/${exei}/${exei}.pj
  $MAKEMSG "Resynchronizing ${projname} ..."
  si resync -R -Y -S ${projname} > /dev/null 2>&1 
  EXESTAT=$?
  if [ "${EXESTAT}" = "0" ]
  then
      $MAKEMSG "Done ($EXESTAT)"
  else
      $MAKEMSG "FAILED ($EXESTAT)"
  fi			
done
BUILDTIME=`date '+%a %b %e %T %Z %Y'`
$MAKEMSG "Resynchronizing finished $BUILDTIME."
