#!/bin/sh
#----------------------------------------------------------------------------
# /usr/local/bin/check-bootfiles.sh
#
# Creation:     2013-03-08 lanspezi
# Last Update:  $Id: check-bootfiles.sh 39751 2015-06-15 21:20:09Z florian $
#----------------------------------------------------------------------------
get_one_var()
{
    local var=$1
    local name=$2
    local file=$3
    eval $var=\"`sed -n -e "s/^$name='\(.*\)'/\1/p" $file`\"
}

check_file_md5()
{
    local rc_cfg=$1
    local name=$2
    local file=$3

    if [ "$file" ]
    then
        var_name=`echo $name | sed -e 's/\./_/g;s/2//' | tr a-z A-Z`
        get_one_var md5sum FLI4L_MD5_$var_name $rc_cfg
        case $file in
            rc*) set `grep -v '^FLI4L_MD5_' $BOOT_DIR/$file | md5sum` ;;
            *)   set `md5sum $BOOT_DIR/$file` ;;
        esac
        real_md5sum=$1
        if [ "$md5sum" -a "$md5sum" != "$real_md5sum" ]
        then
            md5_check="NOK"
            if [ "$VERBOSE" = "YES" ]
            then
                echo "md5sum check for [$BOOT_DIR/$file] failed: $md5sum != $real_md5sum "
            else
                echo "md5sum check for [$BOOT_DIR/$file] failed!"
            fi
        fi
    fi
}

usage ()
{
    echo ""
    echo usage for $0
    echo ""
    echo "$0 [--reboot] [--recover] [--help] [--verbose] [--log-version]"
    echo ""
    echo " --reboot      - reboot the fli4l after a sucessfully md5 check"
    echo " --recover     - check recovery bootfiles instead of standard bootfiles"
    echo " --verbose     - output the different md5sum for every file"
    echo " --log-version - log Version-Infos to syslog"
    echo " --help        - display this help"
    echo ""
}

BOOT_DIR=/boot
boot_files_std='kernel rootfs.img opt.img rc.cfg syslinux.cfg'
boot_files_rec='kernel2 rootfs2.img opt2.img rc2.cfg'
DO_REBOOT="NO"
RECOVER="NO"
VERBOSE="NO"
LOG_VER="NO"
md5_check="OK"


while [ $1 ]
do
    case $1 in
        --reboot)
           DO_REBOOT="YES"
           ;;
        --recover)
           RECOVER="YES"
           ;;
        -?|--help)
           usage
           exit
           ;;
        --verbose)
            VERBOSE="YES"
            ;;
        --log-version)
            LOG_VER="YES"
            ;;
    esac
    shift
done


if [ "$RECOVER" = "YES" ]
then
    echo "checking md5sum of recovery bootfiles"
    for f in $boot_files_rec
    do
        check_file_md5 $BOOT_DIR/rc2.cfg $f $f
    done
else
    echo "checking md5sum of standard bootfiles"
    for f in $boot_files_std
    do
        check_file_md5 $BOOT_DIR/rc.cfg $f $f
    done
fi

if [ "$md5_check" = "OK" ]
then
    echo ""
    echo "md5sum for all checked bootfiles are OK"
    echo ""
    
    if [ "$LOG_VER" = "YES" ]
    then
        mkdir -p /var/lib/persistent/base
        if [ "$RECOVER" = "YES" ]
        then
            get_one_var var_version FLI4L_VERSION   $BOOT_DIR/rc2.cfg
            get_one_var var_date    FLI4L_BUILDDATE $BOOT_DIR/rc2.cfg
            get_one_var var_time    FLI4L_BUILDTIME $BOOT_DIR/rc2.cfg
            get_one_var var_kernel  KERNEL_VERSION $BOOT_DIR/rc2.cfg
            logger -t check-bootfiles.sh[$$] "Recover-Bootfiles updated - VERSION: $var_version KERNEL: $var_kernel BUILT: $var_date $var_time"
            echo "`date '+%Y-%m-%d %T (%Z)'` | update recover bootfiles | $var_version | $var_kernel " >> /var/lib/persistent/base/boot_upd.log
        else
            get_one_var var_version FLI4L_VERSION   $BOOT_DIR/rc.cfg
            get_one_var var_date    FLI4L_BUILDDATE $BOOT_DIR/rc.cfg
            get_one_var var_time    FLI4L_BUILDTIME $BOOT_DIR/rc.cfg
            get_one_var var_kernel  KERNEL_VERSION $BOOT_DIR/rc.cfg
            logger -t check-bootfiles.sh[$$] "Standard-Bootfiles updated - VERSION: $var_version KERNEL: $var_kernel BUILT: $var_date $var_time"
             echo "`date '+%Y-%m-%d %T (%Z)'` | update standard bootfiles | $var_version | $var_kernel " >> /var/lib/persistent/base/boot_upd.log
        fi
    fi
    
    if [ "$DO_REBOOT" = "YES" ]
    then
        echo "please wait - your fli4l router will be restarted now"
        /sbin/reboot
    fi
else
    echo ""
    echo "md5sum of one or more checked files are not OK - please update your bootfiles again"
    echo ""
    for n in 1 2 3 4 5 6 7 8 9 10
    do
        echo -n "."
        sleep 1
    done
    exit 1
fi

