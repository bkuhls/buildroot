# this ulimit is necessary, as dropbear disables core dumps for itself and for
# all of its children, in particular for SSH shells it starts
if [ -f /var/run/coredumps.enabled ]
then
   ulimit -c unlimited
fi

PATH=/sbin:/usr/sbin:/bin:/usr/bin:/usr/local/bin:.; export PATH
version=`cat /etc/version`

PS1="`hostname` $version # "; export PS1
TERM=linux; export TERM

cd /

echo
/usr/local/bin/colecho "Welcome to fli4l !" br x br
fli4l_mode=$(sed -e 's/.*fli4l_mode=\([^[:space:]]\+\).*/\1/' /proc/cmdline)
export fli4l_mode
case $fli4l_mode in
    test | recover)
        /usr/local/bin/colecho "You are running a $fli4l_mode version." br x br
        ;;
esac
echo

if [ -f /etc/.profile ]
then
    . /etc/.profile
fi
if [ -d /etc/profile.d ]
then
    for f in /etc/profile.d/*
    do
        [ -f "$f" ] && . "$f"
    done
fi

if [ -f /bootmsg.txt ]
then
   colecho "There are some warnings or error messages in /bootmsg.txt!" gn
   colecho "Enter \c" gn
   colecho "cat /bootmsg.txt\c" br x br
   colecho "\c"
   colecho " to see the content." gn
   echo
fi
