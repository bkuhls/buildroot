#! /bin/sh
get_parent ()
{
    pid=$1
    case $pid in
        1) res="init (1) -> $res"; return ;;
        *)
            if [ -r /proc/$pid/status ]
            then
                res="`sed -n -e 's/^Name:[[:space:]]*//p' < /proc/$pid/status` ($pid) -> $res"
                get_parent `sed -n -e 's/^PPid:[[:space:]]*//p' < /proc/$pid/status`
            else
                return
            fi
            ;;
    esac
}
res=
pid=$1
: ${pid:=$$}
get_parent $pid
echo $res
