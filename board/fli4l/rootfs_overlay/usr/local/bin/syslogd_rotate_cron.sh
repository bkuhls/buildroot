#------------------------------------------------------------------------------
# syslogd_rotate_cron
#
# Creation:     2009-02-16 LanSpezi
# Last Update:  $Id: syslogd_rotate_cron.sh 17656 2009-10-18 18:39:00Z knibo $
#------------------------------------------------------------------------------

day_old=`date "+%j"`
while [ true ]
do
    day_akt=`date "+%j"`
    if [ $day_old != $day_akt ]
    then
        day_old=$day_akt
        /usr/local/bin/syslogd_rotate.sh
    fi
    sleep 60
done
