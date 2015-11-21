#!sh

if [ "$1" = "Release" ] ; then
	BUILDTYPE=Release
else
	BUILDTYPE=Debug
fi

PATH2BS=${BUILD_SRC_ROOT}
export PATH2BS

msdev serverstart.dsw /useenv /make "ServerStart - Win32 ${BUILDTYPE}" /REBUILD


