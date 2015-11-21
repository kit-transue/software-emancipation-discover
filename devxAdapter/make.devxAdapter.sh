#!/bin/sh
case $1 in 
	Release)
		VERSION='winDevxAdapter - Win32 Release';;
	*)
		VERSION='winDevxAdapter - Win32 Debug';;
esac
msdev winDevxAdapter.dsw /MAKE "$VERSION" $2 $3 $4 $5 $6
#
# -- end:
#
