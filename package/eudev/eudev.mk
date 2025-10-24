################################################################################
#
# eudev
#
################################################################################

EUDEV_VERSION = 3.2.14
EUDEV_SITE = https://github.com/eudev-project/eudev/releases/download/v$(EUDEV_VERSION)
EUDEV_LICENSE = GPL-2.0+ (programs), LGPL-2.1+ (libraries)
EUDEV_LICENSE_FILES = COPYING
EUDEV_INSTALL_STAGING = YES

EUDEV_DEPENDENCIES = host-gperf host-pkgconf
EUDEV_PROVIDES = libudev

EUDEV_CONF_OPTS = \
	--disable-manpages \
	--libexecdir=/lib

ifeq ($(BR2_ROOTFS_MERGED_USR),)
EUDEV_CONF_OPTS += --with-rootlibdir=/lib --enable-split-usr
endif

ifeq ($(BR2_PACKAGE_EUDEV_ENABLE_HWDB),y)
EUDEV_CONF_OPTS += --enable-hwdb
else
EUDEV_CONF_OPTS += --disable-hwdb
endif

ifeq ($(BR2_PACKAGE_EUDEV_DAEMON),y)

# eudev requires only the util-linux libraries at build time
EUDEV_DEPENDENCIES += util-linux-libs
EUDEV_PROVIDES += udev

EUDEV_CONF_OPTS += \
	--enable-programs \
	--sbindir=/sbin \
	--enable-blkid

ifeq ($(BR2_PACKAGE_EUDEV_MODULE_LOADING),y)
EUDEV_CONF_OPTS += --enable-kmod
EUDEV_DEPENDENCIES += kmod
else
EUDEV_CONF_OPTS += --disable-kmod
endif

ifeq ($(BR2_PACKAGE_EUDEV_RULES_GEN),y)
EUDEV_CONF_OPTS += --enable-rule-generator
else
EUDEV_CONF_OPTS += --disable-rule-generator
endif

ifeq ($(BR2_PACKAGE_LIBSELINUX),y)
EUDEV_CONF_OPTS += --enable-selinux
EUDEV_DEPENDENCIES += libselinux
else
EUDEV_CONF_OPTS += --disable-selinux
endif

define EUDEV_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 0755 package/eudev/S10udevd $(TARGET_DIR)/etc/init.d/S10udevd
endef

# Avoid installing S10udevd with openrc, as the service is started by a unit
# from the udev-gentoo-scripts package.
define EUDEV_INSTALL_INIT_OPENRC
	@:
endef

else # !BR2_PACKAGE_EUDEV_DAEMON

EUDEV_CONF_OPTS += \
	--disable-programs \
	--disable-blkid \
	--disable-selinux \
	--disable-kmod \
	--disable-rule-generator

endif

HOST_EUDEV_DEPENDENCIES = host-gperf host-pkgconf

HOST_EUDEV_CONF_OPTS = \
	--prefix=/usr \
	--sbindir=/sbin \
	--libexecdir=/lib \
	--with-rootlibdir=/lib \
	--sysconfdir=/etc \
	--disable-blkid \
	--disable-kmod \
	--disable-manpages \
	--disable-rule-generator \
	--disable-selinux \
	--enable-hwdb

define HOST_EUDEV_INSTALL_CMDS
	$(INSTALL) -D -m 0755 $(@D)/src/udev/udevadm \
		$(HOST_DIR)/bin/udevadm
endef

define HOST_EUDEV_BUILD_HWDB
	$(HOST_DIR)/bin/udevadm hwdb --update --usr --root $(TARGET_DIR)
endef
HOST_EUDEV_TARGET_FINALIZE_HOOKS += HOST_EUDEV_BUILD_HWDB

$(eval $(autotools-package))
$(eval $(host-autotools-package))
