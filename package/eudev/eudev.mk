################################################################################
#
# eudev
#
################################################################################

EUDEV_VERSION = 3.2.2
EUDEV_SITE = http://dev.gentoo.org/~blueness/eudev
EUDEV_LICENSE = GPL-2.0+ (programs), LGPL-2.1+ (libraries)
EUDEV_LICENSE_FILES = COPYING
EUDEV_INSTALL_STAGING = YES

EUDEV_PROVIDES = libudev

ifeq ($(BR2_PACKAGE_EUDEV_DAEMON),y)

# mq_getattr is in librt
EUDEV_CONF_ENV += LIBS=-lrt

EUDEV_CONF_OPTS = \
	--disable-manpages \
	--sbindir=/sbin \
	--libexecdir=/lib \
	--disable-introspection \
	--enable-kmod \
	--enable-blkid

EUDEV_DEPENDENCIES = host-gperf host-pkgconf util-linux kmod
EUDEV_PROVIDES += udev

ifeq ($(BR2_ROOTFS_MERGED_USR),)
EUDEV_CONF_OPTS += --with-rootlibdir=/lib --enable-split-usr
endif

ifeq ($(BR2_PACKAGE_EUDEV_RULES_GEN),y)
EUDEV_CONF_OPTS += --enable-rule-generator
else
EUDEV_CONF_OPTS += --disable-rule-generator
endif

ifeq ($(BR2_PACKAGE_EUDEV_ENABLE_HWDB),y)
EUDEV_CONF_OPTS += --enable-hwdb
else
EUDEV_CONF_OPTS += --disable-hwdb
endif

ifeq ($(BR2_PACKAGE_LIBSELINUX),y)
EUDEV_CONF_OPTS += --enable-selinux
EUDEV_DEPENDENCIES += libselinux
else
EUDEV_CONF_OPTS += --disable-selinux
endif

define EUDEV_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 0755 package/eudev/S10udev $(TARGET_DIR)/etc/init.d/S10udev
endef

# Required by default rules for input devices
define EUDEV_USERS
	- - input -1 * - - - Input device group
endef

else # BR2_PACKAGE_EUDEV_DAEMON

EUDEV_DEPENDENCIES = host-gperf host-pkgconf

EUDEV_CONF_OPTS = \
	--disable-manpages \
	--disable-introspection \
	--disable-blkid \
	--disable-selinux \
	--disable-kmod \
	--disable-hwdb \
	--disable-rule-generator

# When not installing the daemon, we have to override the build and
# install commands, to just install the library.

define EUDEV_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) \
		-C $(@D)/src/shared
	$(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) \
		-C $(@D)/src/libudev
endef

# Symlink udev.pc to libudev.pc for those packages that conflate the two
# and 'Requires: udev' when they should just 'Requires: libudev'. Do the
# symlink, to avoid patching each and all of those packages.
# Note: nothing to install from src/shared, only from src/libudev
define EUDEV_INSTALL_STAGING_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) \
		-C $(@D)/src/libudev DESTDIR=$(STAGING_DIR) install
	ln -sf libudev.pc $(STAGING_DIR)/usr/lib/pkgconfig/udev.pc
endef

define EUDEV_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) \
		-C $(@D)/src/libudev DESTDIR=$(TARGET_DIR) install
endef

endif # BR2_PACKAGE_EUDEV_DAEMON

$(eval $(autotools-package))
