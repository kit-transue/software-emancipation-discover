# Should not need a hashbang to MKS ksh for Windows
#!/bin/ksh

#
# Make an installation
#
# Our InstallShield project requires a hardcoded root; do not change
CDROOTDIR=D:/temp/CIECD
export CDIMAGEDIR=${CDROOTDIR}/Image

# Installshield needs these to be in DOS format, and exported
export NT_CDIMAGEDIR=`echo $CDIMAGEDIR | sed 's/\//\\\\/g'`
export NT_JAVA_HOME=`echo $JAVA_HOME | sed 's/\//\\\\/g'`
export NT_CDROOTDIR=`echo $CDROOTDIR | sed 's/\//\\\\/g'`
export NT_BUILD_SRC_ROOT=`echo $BUILD_SRC_ROOT | sed 's/\//\\\\/g'`

MAKEMSG='echo'

if ( ! test -e ${CDIMAGEDIR} ) ; then
	echo "Making directory ${CDIMAGEDIR}..."
	mkdir -p ${CDIMAGEDIR}
fi

read BUILDNUM < ${BUILD_SRC_ROOT}/makescripts/version
echo "Making ${BUILDNUM} CD..."
#
# making CD image 
#
${MAKEMSG} "Making CD image..."
./install.sh ${CDIMAGEDIR} $1

#
# making install executable 
#
${MAKEMSG} "Making install executable..."

# IS 6.31 bitches about the Setup.Inx file being read-only on uninstall,
# so make it world-readable now, before it becomes part fo the installer
chmod a+w "$BUILD_SRC_ROOT/InstallShield/CodeIntegrityEnterprise/Script Files/Setup.Inx"

PATH="${INSTALL_HOME}/Program;${PATH}"
export PATH
# change this if you want to use a local InstallShield executable
# this may have to be done if you want to build locally, and you do not
# have write privileges on the IS 6.x directory in this default PATH.
# Use an absolute path to ISBuild.exe in this case
ISBUILD=ISBuild.exe
$ISBUILD -mDefault -p"${NT_BUILD_SRC_ROOT}\\InstallShield\\CodeIntegrityEnterprise\\DIScover.ipr" /b${NT_CDROOTDIR}\\Inst\\${BUILDNUM}

${MAKEMSG} "Polishing up the install..."
find ${CDROOTDIR}/Inst/${BUILDNUM} -name '*.pj' -type f -exec rm {} \;

#
# Copy distributable from hard-coded path over to the destination directory
#
#${MAKEMSG} "Moving distributable over to destination tree..."
#cp -R $CDROOTDIR/* $BUILD_SRC_ROOT/CDROM
#rm -rf $CDROOTDIR

${MAKEMSG} "Done!"

#EOF
