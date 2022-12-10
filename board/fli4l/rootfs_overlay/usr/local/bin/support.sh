#! /bin/sh
#----------------------------------------------------------------------------
# support.sh - collect support infos of router
#
# Creation:    17.02.2001 fm
# Last Update: $Id: support.sh 15860 2008-11-05 11:49:51Z arwin $
#----------------------------------------------------------------------------

_MN_err='An error occured.'
_SUP_cfg='config file'
_SUP_nocfg="Cannot access config file (/boot/rc.cfg)!\nThis probably means that you have set MOUNT_BOOT='no'."
_SUP_rccfg='Please recover the config file manually from your boot media.'

exec >/tmp/support.txt 2>&1

head ()
{
    cat <<EOF

#
# $1
#
EOF
}

foot ()
{
    echo
}

cmd ()
{
    if [ 0$# -gt 2 ]
    then
        head "$1 ($3)"
    else
        head "$1 ($2)"
    fi
    $2
    foot
}

show_error ()
{
    echo "$1 $2"
}

head "Support info generated on `date` for fli4l-version `cat /etc/version`"

. /usr/local/bin/support.inc
