#!/bin/sh
case `uname -s` in
	HP-UX) 
		OS="Unix";;
	SunOS)
		OS="Unix";;
	IRIX*) 
		OS="Unix";;
	Linux)
		OS="Unix";;

	*) 
		OS="Windows";;
esac
./${OS}/make.spawnprocess.sh $*
#
# -- end:
#

