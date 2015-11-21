#!sh

if [ "$1" = "Release" ] ; then
	BUILDTYPE=Release
else
	BUILDTYPE=Debug
fi

msdev canceldialog.dsw /useenv /make "CancelDialog - Win32 ${BUILDTYPE}" /REBUILD

