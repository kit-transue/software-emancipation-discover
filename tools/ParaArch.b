#!/bin/csh -f
#

set OS=`uname -s`
set REV=`uname -r` 

if ("$OS" == "SunOS") then
   if ("$REV" == "4.1.1"     \
    || "$REV" == "4.1.2"     \
    || "$REV" == "4.1.3"     \
    || "$REV" == "4.1.3_U1"  \
    || "$REV" == "4.1.3_u1"  \
    || "$REV" == "4.1.3C"    \
   ) then
       arch
   else  if ("$REV" == "5.3") then
       echo sun5
   else
       echo "Os rev not recognized"
       exit 1
   endif
else if ("$OS" == "HP-UX") then
   echo hp700
else
   echo "Machine not known"
   exit 1
endif

exit 0
