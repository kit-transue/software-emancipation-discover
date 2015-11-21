#!sh

PATH2BS=${BUILD_SRC_ROOT}
export PATH2BS

VERSIONNUMBERFILE="../makescripts/market_version"
BUILDNUMBERFILE="../makescripts/version"
ABOUTIMAGE="./discovermdi/res/about.bmp"
BMPWRITER="../makescripts/Windows/BMPWriter.exe"

############ Information for About screen
# First, we must identify the build number and version for the About image

VERSIONNUMBER="`cat ${VERSIONNUMBERFILE}`"
BUILDNUMBER="`cat ${BUILDNUMBERFILE}`"

# Secondly, we write this information to the images

#si co -S /si/Projects/newntui/newntui.pj "${ABOUTIMAGE}"
cp "${ABOUTIMAGE}" "${ABOUTIMAGE}.tmp"
chmod a+w "${ABOUTIMAGE}"
${BMPWRITER} ${ABOUTIMAGE} ${ABOUTIMAGE} -text "Version ${VERSIONNUMBER}  Build ${BUILDNUMBER}" -x 154 -y 202 -size 16 -font Trebuchet

# RTYPE should be Release or Debug
RTYPE=Debug
REBUILD=
opts=$*
for opt in ${opts}
do
	case ${opt} in
	Debug)
		RTYPE=Debug;;
	Release)
		RTYPE=Release;;
	Rebuild)
		REBUILD=/REBUILD;;
	esac
done


# For the time being, because people have been modifying DSPs and DSWs instead
# of the appropriate makefiles, we will build using the DSP.  This should (ideally)
# be rectified at some point in the future, but we need a plan.

NEWNTUIBASE="activex"

# Building OCX's
LIBDIRS="categoriescombo \
        datasource \
        groupscombo \
        coderoverintegrator \
        projectmodules \
        projecttree \
        queriescombo \
        queryresults \
        reporttemolate \
        spreadsheet \
        CMIntegrator \
        Impact"

for DIR in $LIBDIRS
do
	msdev "${NEWNTUIBASE}\\${DIR}\\${DIR}.dsp" /MAKE "${DIR} - Win32 ${RTYPE}" ${REBUILD}
done

DISCOVERMDI=".\discovermdi"
msdev "${DISCOVERMDI}\discovermdi.dsw" /MAKE "DiscoverMDI - Win32 ${RTYPE}" ${REBUILD}

#si revert -Y "${ABOUTIMAGE}"
cp "${ABOUTIMAGE}.tmp" "${ABOUTIMAGE}"

exit

############

#
# location of the source tree, executable, and the name of executable
#
LIBROOT='.'
EXEROOT='./discovermdi'
EXEMAKE='discovermdi'

#
# list of libraries that make discovermdi
#
LIBDIRS="categoriescombo \
        datasource \
        groupscombo \
        coderoverintegrator \
        projectmodules \
        projecttree \
        queriescombo \
        queryresults \
        reporttemolate \
        spreadsheet \
        CMIntegrator \
        Impact"
        

#
# -- begin: invoke the actual build
#
ADMMAKE='../makescripts/Windows'

#
# configure make (default)
#

. $ADMMAKE/make_config.sh

#
# start the driver
#
. $ADMMAKE/make_driver.sh $*

# Let's not forget to unco the about image

#si revert "${ABOUTIMAGE}"
cp "${ABOUTIMAGE}.tmp" "${ABOUTIMAGE}"

# -- end:
