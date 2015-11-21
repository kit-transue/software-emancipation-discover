#!/usr/rd/bin/ksh
#
# $Header: updatebuildnum.ksh 1.4 2003/04/25 11:04:30EDT Chris Bitton (cbitton) Exp  $
#
# Updates the version file with a new build number.  Actual major.minor
# version numbers will need to be changed by hand.
#

# This is planning ahead for when/if we munge build numbers based on
# devpath
REVISION_ARG=head

# Get current build number from latest version file
VERSIONFILE=${LOCAL_SRC_ROOT}/makescripts/version
si co -l --nobranch --changePackageId=:none --nobranchVariant --revision=:${REVISION_ARG} ${VERSIONFILE}
sleep 3
VERSIONSTRING=`cat ${VERSIONFILE}`

# calculate new buildnumber, and reassemble the version string
BUILDNUM=${VERSIONSTRING##*.}
VERSION=${VERSIONSTRING%.*}
let NEW_BUILDNUM=${BUILDNUM}+1
NEW_VERSION=${VERSION}.${NEW_BUILDNUM}

# Overwrite version file and check it in
echo ${NEW_VERSION} > ${VERSIONFILE}
flip -u ${VERSIONFILE}
si ci --nobranch --changePackageId=:none --description="Autoincrement build number from ${BUILDNUM} to ${NEW_BUILDNUM}" ${VERSIONFILE}

#EOF
