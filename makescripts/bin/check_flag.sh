#!/bin/sh

pattern=$1
response=$2

if [ "$pattern" = "" ] ; then
  exit 1
fi

if [ "$response" = "" ] ; then
  response=$pattern
fi

read input_string

if [ "$input_string" = "" ] ; then
  exit 1
fi

echo $input_string | egrep $pattern > /dev/null 2>&1

if [ $? -eq 0 ] ; then
  echo $response
fi

