#! /bin/sh
#------------------------------------------------------------------------------
# syslogd_rotate.sh - rotates syslog-files
# Creation:    LanSpezi 2009-02-16
# Last Update: $Id: syslogd_rotate.sh 50191 2018-01-22 21:20:29Z kristov $
#------------------------------------------------------------------------------

# log destination for all messages that do not match any configured selector
global_protocol_file=/var/log/messages

# Rotates a log file.
# $1 = fully qualified path to the log file
# $2 = maximum number of backups
# Global variables used: $dir_archiv
rotate_logfile()
{
    local logfile=$1 max=$2
    eval syslog_file=`basename $logfile`
    eval dir_source=`dirname $logfile`

    if [ "x$dir_archiv" = "x" ]
    then
        dir_archiv=$dir_source
    fi
    mkdir -p $dir_archiv

    if [ $max -gt 1 ]
    then
        for n in `seq $max -1 2`
        do
            n1=`expr $n - 1`
            if [ -f $dir_archiv/$syslog_file.$n1 ]
            then
                mv $dir_archiv/$syslog_file.$n1 $dir_archiv/$syslog_file.$n
            fi
        done
    fi
    mv $dir_source/$syslog_file $dir_archiv/$syslog_file.1
}

if [ -f /etc/syslog.conf -a -f /var/run/syslogd.pid -o "x$1" = "x--shutdown" ]
then

    #get syslogd-rotate configuration
    . /var/run/syslogd_rotate.conf

    dir_archiv=$SYSLOGD_ROTATE_DIR
    max=$SYSLOGD_ROTATE_MAX
    
    if [ "x$max" = "x" ]
    then
       max=1
    fi

    global_protocol_file_found=
    while read foo syslog_dest
    do
        [ "$syslog_dest" = "$global_protocol_file" ] &&
            global_protocol_file_found=1

        if [ -f $syslog_dest ]
        then
            rotate_logfile $syslog_dest $max
        fi
    done < /etc/syslog.conf

    # rotate /var/log/messages if not done yet and if it exists
    if [ -z "$global_protocol_file_found" -a -f "$global_protocol_file" ]
    then
        rotate_logfile "$global_protocol_file" $max
    fi

    logger -t "syslogd rotate" "syslogfiles are rotated"

fi
