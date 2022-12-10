#!/bin/sh

usage ()
{
    name=`cut -d ' ' -f 3 /var/run/wol/hosts.mac`
    echo ""
    echo "usage 'wol.sh hostname [-b]'"
    echo ""
    echo "list of available hostnames:"
    echo "----------------------------"
    echo $name
    echo ""
}

if [ $# -eq 0 ]
then
    usage
    exit
else
    while [ "$1" ]
    do
        case "$1" in
            -b) bcast="-b" ;;
            *) hname=$1 ;;
        esac
        shift
    done
fi

if [ "x$hname" != "x" ]
then
    if [ -f /var/run/wol/hosts.mac ]
    then
        if grep -q $hname /var/run/wol/hosts.mac
        then
            set `sed -n "/$hname/p" /var/run/wol/hosts.mac`
            /usr/sbin/ether-wake $bcast -i $4 $1
        else
            usage
        fi
    fi
fi
