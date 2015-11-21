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

if [ "$makedir" = "Unix" ] ; then
  ./Unix/make_model_server $*
else 
  if [ "$makedir" = "Windows" ] ; then
    ./Windows/make.model_server.sh $*
  else
    echo "model_server is not supported for the platform."
  fi
fi
