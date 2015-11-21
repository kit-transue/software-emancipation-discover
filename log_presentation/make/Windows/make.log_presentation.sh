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

msdev log_presentation.dsp /useenv /make "log_presentation - Win32 ${BUILDTYPE}" ${REBUILD}

