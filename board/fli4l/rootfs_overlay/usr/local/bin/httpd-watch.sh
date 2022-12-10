. /etc/rc.d/bootmsg-filter-common

check_for_errors ()
{
    if [ -f /bootmsg.txt ]
    then
	local regex="$(httpd_get_bootmsg_regex)"
	[ -n "$(grep -v "$regex" /bootmsg.txt)" ] && return 0
    fi
    for i in /var/log/dumps/*; do
	[ -f "$i" ] && return 0
    done
}

flag=/etc/httpd/errors
check_state ()
{
    if check_for_errors; then
	[ -f $flag ] && return
	httpd-menu.sh add -p 999 problems.cgi '$_MP_problems' '$_MT_stat' status
	> $flag
    else
	[ -f $flag ] || return
	httpd-menu.sh rem problems.cgi
	rm -f $flag
    fi
	
}

while true; do
    check_state
    sleep 60
done
