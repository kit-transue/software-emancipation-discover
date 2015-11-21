#!sh

if [ "$1" = "Release" ] ; then
        BUILDTYPE=Release
else
        BUILDTYPE=Debug
fi

msdev DevXLauncher.dsw /useenv /make "DevXLauncher - Win32 ${BUILDTYPE}" /REBUILD
