#!/bin/sh
#----------------------------------------------------------------------------
# /usr/local/bin/setup-dynamic-iptables.sh - setup dynamic IPv4 packet filter
#
# Last Update:  $Id: setup-dynamic-iptables.sh 37507 2015-03-19 08:11:04Z kristov $
#----------------------------------------------------------------------------

[ -f /var/run/ip_net.conf ] || exit 0

. /etc/boot.d/base-helper
. /etc/rc.d/fwrules-helper

handle_chain()
{
    local table=$1 chain=$2 method=$3 ok=$4

    local processed=
    local delta=0
    local added=0
    iptables -t $table -nL $chain --line-numbers | grep "$dummy_comment \*/" | while read pos rest
    do
        if [ -n "$processed" ]
        then
            iptables -t $table -D $chain $((pos+added-delta))
            delta=$((delta+1))
        elif [ $ok -eq 1 ]
        then
            do_rule "$table" "$chain" R "$rule" $pos $method "$comment"
            added=$((iptables_rules_added-1))
        else
            do_rule "$table" "$chain" R "$dummy_rule" $pos $method "$comment"
            added=$((iptables_rules_added-1))
        fi
        processed=1
    done
}

for i in $(seq 1 $(cat $iptables_dynrules_idx))
do
    rulefile=$iptables_dynrules/$i.rule
    [ -f "$rulefile" ] || continue
    . "$rulefile"

    ok=1
    for ip in $dyn_ips
    do
        if ! can_translate_ip_net $ip
        then
            ok=0
            break
        fi
    done

    handle_chain $table $chain exec_rule $ok
    case $chain in
    PORTFW)
        handle_chain filter PORTFWACCESS exec_access_rule $ok
        handle_chain filter PORTREDIRACCESS exec_access_rule $ok
        ;;
    esac
done
