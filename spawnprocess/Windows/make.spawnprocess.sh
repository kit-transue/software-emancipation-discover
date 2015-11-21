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
 
msdev Windows/spawn_process.dsp /useenv /make "spawn_process - Win32 ${BUILDTYPE}" ${REBUILD}

