#!/bin/sh
#--------------------------------------------------------------------
# /usr/local/bin/httpd-menu.sh - add/remove items from the httpd-menu
#
# Creation:     22.03.2002  tobig
# Last Update:  $Id: httpd-menu.sh 27271 2013-04-23 11:36:46Z sklein $
#--------------------------------------------------------------------

usage ()
{
    echo "Usage: $this add [-p <priority>] <link> <name> [<section>]"
    echo "       $this addsec <priority> <name>"
    echo "       $this rem <link>"
}

show_err ()
{
    . /etc/boot.d/base-helper
    SCRIPT=httpd-menu
    log_error "$1"
}

check_legacy ()
{
    filename=`IFS='?'; set $link; echo $1`
    if [ -f /usr/local/htdocs/$filename ]
    then
        show_err "Using old cgi-script '$filename' with the new-style webgui is deprecated"
        link="index.cgi?link=$link"
    fi
}

add_section() # $priority $name $error
{
    grep -q "^t [0-9]* - $2\$" $menufile
    case $?$3 in
        0yes)
            show_err "section '$2' already exists"
            exit 1
        ;;
        1*)
            # This is one of the ugliest magic pipelines I wrote in years :)
            {
            cat $menufile
            echo -e "t $1 - $2\c"
            } | tr '\n' '\a' | sed 's/\at /\nt /g' | sort | tr '\a' '\n' > $tmp.1
            cp $tmp.1 $menufile
        ;;
    esac
}

# add_menu $link $name $section $priority $right
add_menu ()
{
    add_section 600 "$3" no
    sec="no"
    while read type prio link des right
    do
        [ -n "$right" ] && right=" $right"
        line="$type $prio $link $des$right"
        case $sec in
            no)
                echo "$line"
                case $type$des in
                    "t$3") sec="yes" ;;
                esac
            ;;
            yes)
                case $type in
                    t)
                        sec="no"
                        [ -n "$5" ] && right=" $5" || right=
                        echo -e "${data}e $4 $1 $2$right" | sort
                        echo "$line"
                        data=""
                    ;;
                    e) data="$data$line\n";;
                esac
            ;;
        esac
    done < $menufile > $tmp.2
    case $sec in 
        yes) 
            [ -n "$5" ] && right=" $5" || right=
            echo -e "${data}e $4 $1 $2$right" | sort >> $tmp.2 
        ;; 
    esac
    cp $tmp.2 $menufile
}

# rem_menu $link
rem_menu ()
{
    grep -v "^e [0-9]* $1 " $menufile > $tmp.1
    cp $tmp.1 $menufile
}

#debug
prefix= #/tmp

# initialise Variables
this="`basename $0`"
tmp="/tmp/$this.$$"
menufile="$prefix/etc/httpd/menu"

case "$1" in
    add)
        prio="500"
        case "$2" in
            -p)
                prio="$3"
                shift 2
            ;;
        esac
        link="$2"
        check_legacy $link
        name="$3"
        section="${4:-\$_MT_opt}"
        right="$5"
        add_menu $link $name $section $prio $right
    ;;
    addsec)
        prio="$2"
        name="$3"
        add_section $prio $name yes
    ;;
    rem*)
        link="$2"
        check_legacy $link
        rem_menu $link
    ;;
    *) usage ;;
esac

# remove temp files, invalidate caches
rm -f $tmp.* /tmp/menu_*

# vim: set et sw=4 ts=4:
