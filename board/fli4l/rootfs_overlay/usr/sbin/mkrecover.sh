#!/bin/sh
#----------------------------------------------------------------------------
# /usr/local/bin/mkrecover.sh
# saves or restores the standard installation to/from a recovery version
#
# Creation:     2007-02-19 lanspezi
# Last Update:  $Id: mkrecover.sh 44694 2016-02-28 14:51:32Z lanspezi $
#----------------------------------------------------------------------------

# ---------------
# begin functions
# ---------------
get_free_space ()
{
    # get free size of boot-partition
    set -- $(df /boot | grep boot)
    free=$4
}

get_file_size ()
{
    set -- $(ls -l /boot/$f)
    size=$((($5 - 1) / 1024 + 1))
}

get_one_var()
{
    var=$1
    name=$2
    file=$3
    eval $var=$(sed -n -e "s/^$name='\(.*\)'/\1/p" $file)
}

show_version_info ()
{
    get_one_var version FLI4L_VERSION   $2
    get_one_var date    FLI4L_BUILDDATE $2
    get_one_var time    FLI4L_BUILDTIME $2
    get_one_var kernel  KERNEL_VERSION $2
    get_one_var arch    ARCH $2
    cat <<EOF

$1
    Version: $version
    Date:    $date
    Time:    $time
    Kernel:  $kernel
    Arch:    $arch
EOF
}

copy_file()
{
    if [ -f $1 ]; then
          if ! cp $1 $2; then
              echo "Error copying $3: $1"
              copy_error=1
          fi
    else
          copy_error=1
    fi
}

# ---------------
# begin MAIN
# ---------------

if [ "$1" = "-webgui" -o "$2" = "-webgui" ]
then
    webgui=true
else
    webgui=false
fi

if [ "$1" = "-restore" -o "$2" = "-restore" ]
then
    restore=true
else
    restore=false
fi

if [ ! -d /var/lib/persistent/hd ]
then
    mkdir -p /var/lib/persistent/hd
fi
echo "`date '+%Y-%m-%d %T (%Z)'` | mkrecover.sh called with options: $* " >>/var/lib/persistent/hd/recover.log 

# check whether we are actually running a version we can create a
# recovery version from
mode=$(sed -e 's/.*fli4l_mode=\([^[:space:]]\+\).*/\1/' /proc/cmdline)
if [ "$mode" != normal -a $restore = false ]; then
    cat <<EOF

ERROR: You are running a $mode version, which cannot be used to
       create a recovery version. Please boot a standard version and
       try again.

EOF
    echo "`date '+%Y-%m-%d %T (%Z)'` | mkrecover.sh canceled becouse a recover version is running" >>/var/lib/persistent/hd/recover.log
    exit 1
fi

# check if a restore is available
if [ "$mode" = "normal" -a $restore = true ]
then
    cat <<EOF

ERROR: You are running a $mode version, which cannot be used to
       restore a standard version. Please boot a recovery version and
       try again.

EOF
    echo "`date '+%Y-%m-%d %T (%Z)'` | mkrecover.sh canceled becouse a standard version is running" >>/var/lib/persistent/hd/recover.log
    exit 1
fi

# check mount state of boot partition
if ! grep /boot /proc/mounts | grep -q rw
then
    echo ""
    echo "ERROR: /boot is not mounted read-write - exit"
    echo ""
    grep /boot /proc/mounts
    echo ""
    echo "`date '+%Y-%m-%d %T (%Z)'` | mkrecover.sh canceled becouse /boot is readonly" >>/var/lib/persistent/hd/recover.log
    exit 1
fi

standard_files="kernel rootfs.img rc.cfg opt.img"
recovery_files="kernel2 rootfs2.img rc2.cfg opt2.img"

if [ $restore = false ]
then
#
# create a recovery version from standard version
#
    get_free_space

    needed=0
    for f in $standard_files opt.old
    do
          if [ -f /boot/$f ]
          then
              get_file_size /boot/$f
              needed=$((needed + size))
          fi
    done

    # get size used by old recovery version
    usedby=0
    for f in $recovery_files
    do
          if [ -f /boot/$f ]
          then
              get_file_size /boot/$f
              usedby=$((usedby + size))
          fi
    done

    free2=$((free + usedby))

    if [ "$webgui" != "true" ]; then
          show_version_info "Current Version" /boot/rc.cfg
          [ -f /boot/rc2.cfg ] && show_version_info "Recovery Version" /boot/rc2.cfg
    fi

    cat <<EOF

    Space free on /boot                : $free
    Space needed                       : $needed
    Space used by old recovery version : $usedby
    Space available                    : $free2
        (available after removing old recovery version)

EOF

    if [ "$needed" -gt "$free2" ]
    then
          echo "ERROR: free space on /boot is too low - exit"
          echo ""
          echo "`date '+%Y-%m-%d %T (%Z)'` | mkrecover.sh canceled becauce low space on /boot" >>/var/lib/persistent/hd/recover.log
          exit 1
    fi

    if [ $usedby -gt 0 -a "$webgui" != "true" ]
    then
        echo ""
        echo "Old recovery installation found!"
        echo ""
        echo "To overwrite answer YES"
        read answer
        if [ "$answer" != "YES" ]
        then
            echo ""
            echo "You cancelled the process - no change to recovery"
            echo "`date '+%Y-%m-%d %T (%Z)'` | mkrecover.sh canceled by operator" >>/var/lib/persistent/hd/recover.log
            exit 1
        fi
    fi

    copy_error=
    for i in $standard_files
    do
        [ -f /boot/$i ] && continue

        case $i in
            opt.img) [ -f /boot/opt.old ] && continue ;;
        esac
        echo "Missing /boot/$i ..."
        copy_error=1
    done

    if [ "$copy_error" ]; then
        echo ""
        echo "Aborting creation of recovery version ..."
        echo ""
        echo "`date '+%Y-%m-%d %T (%Z)'` | mkrecover.sh canceled becauce missing files on /boot" >>/var/lib/persistent/hd/recover.log
        exit 1
    fi

    # remove old recovery version to free up space
    for i in $recovery_files
    do
        rm -f /boot/$i
    done

    copy_file /boot/kernel     /boot/kernel2      "kernel image"
    copy_file /boot/rc.cfg     /boot/rc2.cfg      "configuration file"
    copy_file /boot/rootfs.img /boot/rootfs2.img  "ROOTFS image"
    if [ -f /boot/opt.img ]; then
        copy_file /boot/opt.img /boot/opt2.img "OPT image"
    else
        copy_file /boot/opt.old /boot/opt2.img "OPT image"
    fi
    if [ -f /boot/boot_s.msg ]; then
       # create boot_r.msg -> menu entry for recovery version in boot menu
       echo "r <=> boot fli4l recovery installation" > /boot/boot_r.msg
       get_one_var version FLI4L_VERSION /boot/rc2.cfg
       echo "      Version: $version" >> /boot/boot_r.msg
       echo "" >> /boot/boot_r.msg
    fi

    if [ "$copy_error" ]; then
        echo ""
        echo "failed to create recovery version ..."
        echo ""
        exit 1
    else
        echo ""
        echo "recovery installation was created from standard version...."
        echo ""
        get_one_var version FLI4L_VERSION /boot/rc.cfg
        echo "`date '+%Y-%m-%d %T (%Z)'` | recovery-version created from standard-version $version" >>/var/lib/persistent/hd/recover.log
    fi

else
#
# restore standard version from booted recovery version
#
    copy_error=
    for i in $recovery_files
    do
        [ -f /boot/$i ] && continue

        echo "Missing /boot/$i ..."
        copy_error=1
    done

    if [ "$copy_error" ]; then
        echo ""
        echo "Restoring standard installation aborted ..."
        echo ""
        exit 1
    fi

    # remove old normal version to free up space
    for i in $standard_files opt.old
    do
        rm -f /boot/$i
    done

    copy_file /boot/kernel2     /boot/kernel      "kernel image"
    copy_file /boot/rc2.cfg     /boot/rc.cfg      "configuration file"
    copy_file /boot/rootfs2.img /boot/rootfs.img  "ROOTFS image"
    copy_file /boot/opt2.img    /boot/opt.img     "OPT image"
        
    if [ -f /boot/boot_r.msg ]
    then
       # create boot_s.msg -> menu entry for standard version in boot menu
       echo "n <=> boot fli4l standard installation" > /boot/boot_s.msg
       get_one_var version FLI4L_VERSION /boot/rc2.cfg
       echo "      Version: $version" >> /boot/boot_s.msg
       echo "" >> /boot/boot_s.msg
    fi

    if [ "$copy_error" ]
    then
        echo ""
        echo "failed to replace standard installation by recovery version ..."
        echo ""
        exit 1
    else
        echo ""
        echo "standard installation replaced by recovery version...."
        echo ""
        get_one_var version FLI4L_VERSION /boot/rc2.cfg
        echo "`date '+%Y-%m-%d %T (%Z)'` | standard-version restored from recovery-version $version" >>/var/lib/persistent/hd/recover.log
    fi

fi
