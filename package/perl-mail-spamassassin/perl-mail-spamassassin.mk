################################################################################
#
# perl-mail-spamassassin
#
################################################################################

PERL_MAIL_SPAMASSASSIN_VERSION = 3.4.4
PERL_MAIL_SPAMASSASSIN_SOURCE = Mail-SpamAssassin-$(PERL_MAIL_SPAMASSASSIN_VERSION).tar.gz
PERL_MAIL_SPAMASSASSIN_SITE = $(BR2_CPAN_MIRROR)/authors/id/S/SI/SIDNEY
PERL_MAIL_SPAMASSASSIN_DEPENDENCIES = perl-digest-sha1 perl-html-parser perl-mail-dkim perl-net-dns perl-netaddr-ip
PERL_MAIL_SPAMASSASSIN_LICENSE = Apache-2.0
PERL_MAIL_SPAMASSASSIN_LICENSE_FILES = LICENSE

ifeq ($(BR2_PACKAGE_PERL_DB_FILE),y)
PERL_MAIL_SPAMASSASSIN_DEPENDENCIES += perl-db-file
endif

ifeq ($(BR2_PACKAGE_PERL_ENCODE_DETECT),y)
PERL_MAIL_SPAMASSASSIN_DEPENDENCIES += perl-encode-detect
endif

ifeq ($(BR2_PACKAGE_PERL_HTTP_DATE),y)
PERL_MAIL_SPAMASSASSIN_DEPENDENCIES += perl-http-date
endif

ifeq ($(BR2_PACKAGE_PERL_IO_SOCKET_SSL),y)
PERL_MAIL_SPAMASSASSIN_DEPENDENCIES += perl-io-socket-ssl
endif

ifeq ($(BR2_PACKAGE_PERL_LIBWWW_PERL),y)
PERL_MAIL_SPAMASSASSIN_DEPENDENCIES += perl-libwww-perl
endif

PERL_MAIL_SPAMASSASSIN_CONF_OPTS += \
	VENDORPREFIX=/usr \
	INSTALLVENDORCONF=/etc/mail/spamassassin \
	CONTACT_ADDRESS="$(call qstrip,$(BR2_PACKAGE_PERL_MAIL_SPAMASSASSIN_CONTACT_ADDRESS))"

$(eval $(perl-package))
