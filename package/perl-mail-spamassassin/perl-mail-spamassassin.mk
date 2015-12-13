################################################################################
#
# perl-mail-spamassassin
#
################################################################################

PERL_MAIL_SPAMASSASSIN_VERSION = 4.0.1
PERL_MAIL_SPAMASSASSIN_SOURCE = Mail-SpamAssassin-$(PERL_MAIL_SPAMASSASSIN_VERSION).tar.bz2
PERL_MAIL_SPAMASSASSIN_SITE = https://downloads.apache.org/spamassassin/source
PERL_MAIL_SPAMASSASSIN_LICENSE = Apache-2.0
PERL_MAIL_SPAMASSASSIN_LICENSE_FILES = LICENSE
PERL_MAIL_SPAMASSASSIN_CONF_OPTS += \
	VENDORPREFIX=/usr \
	INSTALLVENDORCONF=/etc/mail/spamassassin \
	CONTACT_ADDRESS="$(call qstrip,$(BR2_PACKAGE_PERL_MAIL_SPAMASSASSIN_CONTACT_ADDRESS))"

$(eval $(perl-package))
