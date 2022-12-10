#!/bin/sh

mkdir -p /var/run/arping.stat
hostsn=`sed -n '$=' /var/run/arping.ip`
logger -t arping[$$] -p local1.info "start host state monitoring for $hostsn hosts"

# maximum number of cores
# (avoid parsing /proc/cpuinfo, access sysfs instead)
maxcpus=$(ls -1d /sys/devices/system/cpu/cpu* | sed -E '/cpu[0-9]+$/!d' | wc -l)

# maximum number of hosts checked in parallel
maxhosts=$((maxcpus*3+1))

host_online ()
{
    hn=$1
    if [ ! -f /var/run/arping.stat/${hn}.online ]
    then
       echo "online" > /var/run/arping.stat/${hn}.online
       logger -t arping[$$] -p local1.info "host '${hn}' is now reachable"
    fi
}
host_offline ()
{
    hn=$1
    ip=$2
    dev=$3
    ## Remove entry from arp-table!
    arp -d ${ip} -i ${dev} > /dev/null 2>&1
    if [ -f /var/run/arping.stat/${hn}.online ]
    then
        rm /var/run/arping.stat/${hn}.online
        logger -t arping[$$] -p local1.info "host '${hn}' is no longer reachable"
    fi
}

check_host ()
{
    ipaddr1=$1
    ipdev1=$2
    hname1=$3
    arping -f -c 9 -w 2 -b -q ${ipaddr1} -I ${ipdev1}
    if [ $? != 0 ]
    then
        host_offline ${hname1} ${ipaddr1} ${ipdev1}
    else
        ping -c 2 ${ipaddr1} > /dev/null 2>&1
        host_online ${hname1}
    fi
}


while true
do
    logger -t arping[$$] -p local1.debug "start of loop"
    anz=0
    while read ipaddr ipdev hname
    do
        check_host $ipaddr $ipdev $hname &
        anz=$((anz+1))
        if [ $anz -eq $maxhosts ]
        then
            # wait for unfinished checks
            wait
            anz=0
        fi
    done < /var/run/arping.ip
    # wait for unfinished checks in case the total number of hosts is not
    # dividable by $maxhosts
    wait
    logger -t arping[$$] -p local1.debug "end of loop"

    # sleep one minute, then try again
    sleep 60
done
