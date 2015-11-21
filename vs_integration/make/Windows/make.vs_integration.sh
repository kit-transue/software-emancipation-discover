#!sh

PATH2BS=${BUILD_SRC_ROOT}
export PATH2BS

if [ "x$1" = "xRelease" ] ; then
	CFG="VSAddIn - Win32 Release"
else
	CFG="VSAddIn - Win32 Debug"
fi

msdev VSAddIn.dsp /useenv /make "$CFG" /REBUILD
