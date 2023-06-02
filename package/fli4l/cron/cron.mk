################################################################################
#
# cron
#
################################################################################

CRON_VERSION = 3.0pl1
CRON_SOURCE = cron_$(CRON_VERSION).orig.tar.gz
CRON_PATCH = cron_$(CRON_VERSION)-196.debian.tar.xz
CRON_SITE = https://snapshot.debian.org/archive/debian/20250406T210903Z/pool/main/c/cron
CRON_LICENSE = Proprietary
CRON_LICENSE_FILES = README

CRON_MAKE_OPTS = \
	$(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS) -DDEBUGGING=0 -I."

define CRON_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(CRON_MAKE_OPTS) -C $(@D)
endef

define CRON_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/cron $(TARGET_DIR)/usr/sbin/cron
	$(INSTALL) -D -m 0755 $(@D)/crontab $(TARGET_DIR)/usr/bin/crontab
endef

$(eval $(generic-package))
