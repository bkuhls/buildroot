################################################################################
#
# e3
#
################################################################################

E3_VERSION = 2.82
E3_SITE = https://gentoo.osuosl.org/distfiles/9a
E3_SOURCE = e3-$(E3_VERSION).tgz
E3_LICENSE = GPL-2.0+
E3_LICENSE_FILES = COPYING.GPL
E3_DEPENDENCIES = host-nasm

define E3_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) \
		$(if $(BR2_ARCH_IS_64),64,32)
endef

define E3_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TARGET_CONFIGURE_OPTS) \
		PREFIX="$(TARGET_DIR)/usr" install
endef

$(eval $(generic-package))
