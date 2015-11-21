#!sh

if [ "$1" = "Release" ] ; then
	BUILDTYPE=Release
else
	BUILDTYPE=Debug
fi

OUTDIR="./${BUILDTYPE}"
COMPILER="${JAVA_HOME}/bin/javac"
JAR="${JAVA_HOME}/bin/jar"

ADMMAKE=`pwd`/../makescripts/Windows
PATH2BS=${BUILD_SRC_ROOT}

# Manifest stuff
MANIFEST="JBuilderAddInManifest.txt"
TEMPMANIFEST="jbManifest.txt"
VERSIONNUMBER="`cat ${ADMMAKE}/../market_version`"
BUILDNUMBER="`cat ${ADMMAKE}/../version`"

echo "Removing old class files..."
if [ -d "${OUTDIR}" ]
then
	rm -rf "${OUTDIR}" 
fi

mkdir -p "${OUTDIR}"

echo "Compiling JBuilderAddIn..."
JBUILDERCLASSPATH=../thirdparty/jbuilder/jbuilder.jar
if [ ! -f ${JBUILDERCLASSPATH} ] ; then
	JBUILDERCLASSPATH=${PATH2BS}/thirdparty/jbuilder/jbuilder.jar
	if [ ! -f ${JBUILDERCLASSPATH} ] ; then
		JBUILDERCLASSPATH=
	fi
fi

${COMPILER}  -d ${OUTDIR} -classpath ".;${JBUILDERCLASSPATH}" -sourcepath "./src;../CodeRover;${PATH2BS}/CodeRover" ./src/JBuilderAddIn/AddInMain.java

COMPSTAT=$?
if [ $COMPSTAT = 0 ] ; then
	mkdir -p ${OUTDIR}/JBuilderAddIn/images
	cp images/* ${OUTDIR}/JBuilderAddIn/images
	
	echo "Jar'ing JBuilderAddIn.jar file..."
	cp "${MANIFEST}" "${TEMPMANIFEST}"
	
	echo "BuildNumber: ${BUILDNUMBER}" >> "${TEMPMANIFEST}"
	echo "VersionNumber: ${VERSIONNUMBER}" >> "${TEMPMANIFEST}"

	CURDIR=`pwd`
	cd "${OUTDIR}"
	${JAR} -cfm JBuilderAddIn.jar "${CURDIR}/${TEMPMANIFEST}" *

	cd "${CURDIR}"
	rm "${TEMPMANIFEST}"
	echo "JBuilderAddIn creation complete."
else
	echo "Failed to compile JBuilderAddIn"
	exit 1
fi

