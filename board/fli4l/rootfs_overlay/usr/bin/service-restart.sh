#!/bin/sh
#--------------------------------------------------------------------
# /usr/local/bin/service-restart.sh - Run service in a loop
#
# Creation:    08.02.2003 tobig
# Last Update: $Id: service-restart.sh 49161 2017-11-12 20:48:01Z kristov $
#--------------------------------------------------------------------

SEM=/var/run/service-restart.exit

case $1 in
    start) rm -f $SEM ; exit ;;
    stop)  > $SEM ; exit ;;
esac

if [ "$#" -lt 2 ]
then
  echo "Syntax: $0 <sleeptime> <programm> [arg1] ..."
  exit 1
fi

sleeptime="$1"
shift

if echo "$sleeptime" | grep -v "^[0-9]\+$" > /dev/null
then
  echo "Error: Sleep time is not a number!"
  exit 1
fi

if [ ! -x "$1" ]
then
  echo "Error: service not found or not executable!"
  exit 1
fi

trap "" 1

while true
do
  "$@"
  rc=$?
  if [ -f $SEM ]; then
      logger -t "$0[$$]" "$1 terminated with $rc, not restarting due to system shutdown ..."
      exit
  fi
  # logger -t "$0[$$]" "$1 terminated with $rc, restarting in $sleeptime seconds"
  sleep "$sleeptime"
done
