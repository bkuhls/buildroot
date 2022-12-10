#! /bin/sh

new_param=none
log=log_error

case $0 in
    ps | *bin/ps)
        case x$1 in
            x | xw | xww | xwww)   WRAP_SILENT=yes ;;
            *)  new_param=
        esac
        ;;
    ip | */ip)
	trace=yes
        log=log_info
	;;
esac

if [ -z "$WRAP_SILENT" ]; then
    if [ -x /usr/local/bin/get_tree.sh ]; then
        pid=$$
        msg="'$0 $*' called, call sequence: `/usr/local/bin/get_tree.sh $pid` $0"
        ppid=`sed -n -e 's/^PPid:[[:space:]]*//p' < /proc/$pid/status`
        pname=`sed -n -e 's/^Name:[[:space:]]*//p' < /proc/$ppid/status`
        if [ "$trace" = yes ] || ! grep -q "^$pname - $0 $@" /var/log/wrapper.log; then
            echo "$pname - $0 $@" >> /var/log/wrapper.log
            . /etc/boot.d/base-helper
            SCRIPT=$0
            case x$script in
                x) $log "$msg" ;;
                *) $log "$msg, sub script $script" ;;
            esac
        fi
    fi
fi

case "$new_param" in
    none) ;;
    *) set "$new_param" ;;
esac

if [ -f /usr/local/bin/wrapped/`basename $0` ]
then
    exec /usr/local/bin/wrapped/`basename $0` $*
else
    . /etc/boot.d/base-helper
    log_error "`basename $0` does not exist anymore - please read documentation"
fi
