################################################################################
#
# libudev-zero
#
################################################################################

LIBUDEV_ZERO_VERSION = 0.5.0
LIBUDEV_ZERO_SITE = $(call github,illiliti,libudev-zero,$(LIBUDEV_ZERO_VERSION))
LIBUDEV_ZERO_LICENSE = ISC
LIBUDEV_ZERO_LICENSE_FILES = LICENSE
LIBUDEV_ZERO_INSTALL_STAGING = YES

define LIBUDEV_ZERO_BUILD_CMDS
	$(TARGET_MAKE_ENV) AR="$(TARGET_AR)" CC=$(TARGET_CC) $(MAKE) -C $(@D)
endef

define LIBUDEV_ZERO_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) DESTDIR=$(TARGET_DIR) PREFIX=/usr install
endef

define LIBUDEV_ZERO_INSTALL_STAGING_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) DESTDIR=$(STAGING_DIR) PREFIX=/usr install
endef

$(eval $(generic-package))
