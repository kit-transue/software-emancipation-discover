#!/bin/sh

makedir=""

case `uname -s` in
	SunOS)
		makedir="Unix";;
	HP-UX)
		makedir="Unix";;
	IRIX*)
		makedir="Unix";;
	Linux)
		makedir="Unix";;
	*)
		makedir="unknown";;
esac

if [ "$makedir" = "Unix" ] ; then
  cd ./make/Unix
  ./make_aset $*
else 
  echo "aset is not supported for the platform."
fi
