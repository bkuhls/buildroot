#!/bin/sh

# delete various links
for i in \
    ct_run \
    dialyzer \
    dsync \
    epmd \
    erl \
    erlc \
    escript \
    run_erl \
    to_erl \
    typer \
    python \
    python3 \
    git-receive-pack \
    git-upload-archive \
    ; \
do
    if [ -L $1/usr/bin/$i ]; then \
        rm -fv $1/usr/bin/$i;
    fi
done

# link dovecot libraries from subdirs to /usr/lib/
for i in \
    lib01_acl_plugin.so \
    lib10_mail_crypt_plugin.so \
    lib10_quota_plugin.so \
    lib15_notify_plugin.so \
    lib20_fts_plugin.so \
    lib90_old_stats_plugin.so \
    lib95_imap_old_stats_plugin.so \
    ; \
do
    if [ ! -L $1/usr/lib/$i ]; then \
        ln -svf dovecot/$i $1/usr/lib/$i
    fi
done
ln -svf dovecot/old-stats/libstats_auth.so $1/usr/lib/libstats_auth.so

# remove link to dovecot binary
rm -fv $1/usr/libexec/deliver

cp -fv $1/sbin/e2fsck $1/usr/sbin/e2fsck
cp -fv $1/sbin/xtables-legacy-multi $1/sbin/xtables-multi

rm -fv $1/lib/xtables/libxt_DNAT.so
rm -fv $1/lib/xtables/libxt_MASQUERADE.so
rm -fv $1/lib/xtables/libxt_NOTRACK.so
rm -fv $1/lib/xtables/libxt_REDIRECT.so
rm -fv $1/lib/xtables/libxt_SNAT.so

cp -fv $1/usr/bin/nc $1/usr/bin/netcat

ln -svf ../libexec/sudo/libsudo_util.so.0 $1/usr/lib/libsudo_util.so.0

find $1/usr/libexec/git-core -type l -exec rm -v {} +

for i in `find $1/usr/share/kodi/addons -type f | grep language | grep -v "xsd\|de_de\|en_gb\|English\|German"`; do rm -vf $i; done

for i in `find $1/usr/lib/perl5 -name *.pod`; do rm -vf $i; done

for i in `find $1/usr/share/zoneinfo -type l`; do rm -vf $i; done

eval $(make -s printvars VARS=BR2_LINUX_KERNEL_VERSION)
cp -fv $1/boot/bzImage $1/boot/kernel-$BR2_LINUX_KERNEL_VERSION
