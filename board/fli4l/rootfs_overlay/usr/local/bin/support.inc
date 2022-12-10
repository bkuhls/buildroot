#! /bin/sh
#----------------------------------------------------------------------------
# support.inc - collect support infos of router
#
# Creation:    17.02.2001 fm
# Last Update: $Id: support.inc 35241 2014-12-02 20:13:36Z kristov $
#----------------------------------------------------------------------------

show_imond_conf ()
{
    while read line
    do
        set $line
        output="$1 $2 $3 $4 $5 $6 $7 xxxx"
        shift 8
        output="$output $1 $2 $3 $4"
        echo $output
    done < /etc/imond.conf
}

show_telmond_conf ()
{
    sed 's/^[0-9][0-9]*/xxxx/' </etc/telmond.conf
}
show_iwconfig ()
{
    for dev in `iwconfig 2>/dev/null | sed -n '/^wlan[0-9]/s/ .*$//p'`
    do
        iwconfig $dev | sed '/Encryption/s/-[0-9A-F][0-9A-F]/-XX/g;/Encryption/s/[0-9A-F][0-9A-F]-/XX-/g;s/\(Encryption key:\)[0-9A-F][0-9A-F]/\1XX/'
    done
}
show_rc_cfg ()
{
    if [ -f /boot/rc.cfg ]
    then
        sed "s/^\([A-Z0-9_]*PASS[A-Z0-9_]*\)='.*'/\1='xxxx'/
             s/^\([A-Z0-9_]*USER[A-Z0-9_]*\)='.*'/\1='xxxx'/
             s/^\([A-Z0-9_]*CIRC_[0-9][0-9]*_DIAL[A-Z0-9_]*\)='.*'/\1='xxxx'/
             s/^\([A-Z0-9_]*CIRC_[0-9][0-9]*_EAZ\)='.*'/\1='xxxx'/
             s/^\(TELMOND_CMD_[0-9][0-9]*\)='[0-9][0-9]*/\1='xxxx/
             s/^\([A-Z0-9_]*_MSN_*[0-9]*\)='[0-9][0-9]*/\1='xxxx/
             s/^\(WLAN_ENC_[0-9]*\)='.*'/\1='xxxx'/
             s/^\(WLAN_[0-9]*_WPA_PSK\)='.*'/\1='xxxx'/
            " /boot/rc.cfg
    else
        echo "$_MN_err"
        echo -e "$_SUP_nocfg"
        echo -e "$_SUP_rccfg"
    fi
}
show_isdn_if ()
{
        /sbin/isdnctrl list all | sed "s+^\(EAZ/MSN: *\).*+\1xxxx+
        s+\(Outgoing: *\).*+\1xxxx+
        s+\(Incoming: *\).*+\1xxxx+" 
}
show_packages ()
{
    if [ -f /boot/rc.cfg ]
    then
        grep -e "\(# package\)\|\(OPT_.*='yes'\)" /boot/rc.cfg | sed -e "s/\(='yes'\)\|\(# \)//g"
    else
        echo "$_MN_err"
        echo -e "$_SUP_nocfg"
        echo -e "$_SUP_rccfg"
    fi
}
show_pci_dev ()
{
    cat /proc/pci /proc/bus/pci/devices
    if [ -f /usr/bin/lspci ]
    then
        echo -e "\n(/usr/bin/lspci -vnnk)\n"
        /usr/bin/lspci -vnnk
    fi
}

cmd "$_SUP_ps"  '/bin/ps ww'
#if [ -f /bin/netstat ]
#then
#    cmd "$_SUP_ifcfg"       'netstat -ei'
#else
    cmd "$_SUP_ifcfg"       'ip addr show'
#fi
[ -f /sbin/ip6tables ] && cmd "$_SUP_ifcfg6" 'ip -6 addr show'
#if [ -f /bin/netstat ]
#then
#    cmd "$_SUP_route"       'netstat -er'
#else
    cmd "$_SUP_route"       'ip route show'
#fi
[ -f /sbin/ip6tables ] && cmd "$_SUP_route6" 'ip -6 route'
cmd "$_SUP_df"  '/bin/df -h'
cmd "$_SUP_meminf"      'cat /proc/meminfo'
cmd "$_MP_pf (filter)"  'iptables -nvL'
if grep -q ^nat /proc/net/ip_tables_names
then
    cmd "$_MP_pf (nat)" 'iptables -t nat -nvL'
fi
if grep -q ^mangle /proc/net/ip_tables_names
then
    cmd "$_MP_pf (mangle)"      'iptables -t mangle -nvL'
fi
[ -f /sbin/ip6tables ] && cmd "$_MP_pf (IPv6)"  'ip6tables -nvL'

if [ -s /etc/imond.conf ]
then
    cmd "imond $_SUP_cfg" 'show_imond_conf' '/etc/imond.conf'
fi

if [ -f /etc/telmond.conf ]
then
    cmd "telmond $_SUP_cfg" 'show_telmond_conf' '/etc/telmond.conf'
fi

if [ -f /sbin/iwconfig ]
then
    cmd "$_SUP_wlan" 'show_iwconfig' 'iwconfig'
fi

cmd "$_SUP_cfg" 'show_rc_cfg' ''

cmd "$_SUP_arch"    'uname -m'

cmd "$_SUP_kver"        'cat /proc/version'
if [ -f /bin/lsmod ]
then
    cmd "$_SUP_mod"         'lsmod'
else
    cmd "$_SUP_mod"         'cat /proc/modules'
fi
if grep -q "version 2.4" /proc/version
then
    cmd "$_SUP_ksyms"       'cat /proc/ksyms'
else
    cmd "$_SUP_ksyms"       'cat /proc/kallsyms'
fi
cmd "$_SUP_pci"         'show_pci_dev'
cmd "$_SUP_irq"         'cat /proc/interrupts'
cmd "$_SUP_io"          'cat /proc/ioports'
cmd "$_SUP_cpu"         'cat /proc/cpuinfo'

if [ -f /etc/isapnp.conf ]
then
    cmd "isapnp $_SUP_cfg" 'cat /etc/isapnp.conf'
fi

cmd "$_SUP_packages"    'show_packages' 
cmd "$_SUP_netdrivers"  'cat /var/run/netdrivers.conf'

if [ -f /sbin/isdnctrl ]
then
    cmd "$_SUP_isdnif" 'show_isdn_if' 'isdnctrl list all'
fi

cmd "$_SUP_netaliases"  'cat /var/run/netaliases.conf'
