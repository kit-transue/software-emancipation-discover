#!/bin/sh

BUILDTYPE=Debug
REBUILD=
opts=$*
for opt in ${opts}
do
	case ${opt} in
	Debug)
		BUILDTYPE=Debug;;
	Release)
		BUILDTYPE=Release;;
	Rebuild)
		REBUILD=/REBUILD;;
	esac
done
 
msdev mbdriver.dsp /useenv /make "mbdriver - Win32 ${BUILDTYPE}" ${REBUILD}

