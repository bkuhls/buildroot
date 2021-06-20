################################################################################
#
# openrgb
#
################################################################################

OPENRGB_VERSION = release_0.6
OPENRGB_SITE = $(call gitlab,CalcProgrammer1,OpenRGB,$(OPENRGB_VERSION))
OPENRGB_LICENSE = GPL-2.0+
OPENRGB_LICENSE_FILES = LICENSE
OPENRGB_DEPENDENCIES = \
	hidapi \
	qt5base

OPENRGB_CONF_ENV = \
	PKG_CONFIG_PATH="$(STAGING_DIR)/usr/lib/pkgconfig"

$(eval $(qmake-package))
