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
	Windows_NT)
		makedir="Windows";;
	*)
		makedir="unknown";;
esac

if [ "$makedir" = "Windows" ] ; then
  ./Windows/make.pset_server.sh $*
else
  echo "pset_server is not supported for the platform."
fi
