################################################################################
#
# dsltool
#
################################################################################

DSLTOOL_LICENSE = GPL-2.0+
DSLTOOL_LICENSE_FILES = COPYING

DSLTOOL_DEPENDENCIES = host-pkgconf cairo pango collectd

define DSLTOOL_EXTRACT_CMDS
	cp package/fli4l/dsltool/src/* $(@D)/
endef

define DSLTOOL_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC=$(TARGET_CC) LD=$(TARGET_LD) CFLAGS="$(TARGET_CFLAGS)" \
		-C $(@D) build
endef

define DSLTOOL_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) INSTALL="$(INSTALL)" TARGET_DIR="$(TARGET_DIR)/usr/bin" \
		-C $(@D) install
endef

$(eval $(generic-package))
