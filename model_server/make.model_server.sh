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
  cd ./make/Unix
  ./make_model_server $*
else 
  if [ "$makedir" = "Windows" ] ; then
    # ./make/Windows/make.model_server.sh should be updated to be
    # run from its containing directory.
    cd ./make
    ./Windows/make.model_server.sh $*
  else
    echo "model_server is not supported for the platform."
  fi
fi
