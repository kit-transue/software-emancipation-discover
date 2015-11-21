#!sh

PATH2BS=${BUILD_SRC_ROOT}
export PATH2BS

if [ "$1" = "Release" ] ; then
	BUILDTYPE=Release
else
	BUILDTYPE=Debug
fi

msdev emacs_integration.dsw /useenv /make "server - Win32 ${BUILDTYPE}" /REBUILD
msdev emacs_integration.dsw /useenv /make "client - Win32 ${BUILDTYPE}" /REBUILD

