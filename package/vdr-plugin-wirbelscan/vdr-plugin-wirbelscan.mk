################################################################################
#
# vdr-plugin-wirbelscan
#
################################################################################

VDR_PLUGIN_WIRBELSCAN_VERSION = 0.0.9
VDR_PLUGIN_WIRBELSCAN_SOURCE = vdr-wirbelscan-$(VDR_PLUGIN_WIRBELSCAN_VERSION).tgz
VDR_PLUGIN_WIRBELSCAN_SITE = http://wirbel.htpc-forum.de/wirbelscan
VDR_PLUGIN_WIRBELSCAN_LICENSE = GPLv2+
VDR_PLUGIN_WIRBELSCAN_LICENSE_FILES = COPYING
VDR_PLUGIN_WIRBELSCAN_DEPENDENCIES = vdr w_scan

define VDR_PLUGIN_WIRBELSCAN_FIX_MAKEFILE
	$(SED) '/Make.global/d' $(@D)/Makefile
endef
VDR_PLUGIN_WIRBELSCAN_POST_PATCH_HOOKS += VDR_PLUGIN_WIRBELSCAN_FIX_MAKEFILE

VDR_PLUGIN_WIRBELSCAN_CXXFLAGS = \
	CXXFLAGS="$(TARGET_CXXFLAGS) -fPIC" \

define VDR_PLUGIN_WIRBELSCAN_BUILD_CMDS
	mkdir -p $(TARGET_DIR)/usr/lib/vdr
	$(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D) \
		$(VDR_PLUGIN_WIRBELSCAN_CXXFLAGS) \
		VDRDIR=$(STAGING_DIR)/usr/include/vdr \
		LIBDIR=$(TARGET_DIR)/usr/lib/vdr \
		libvdr-wirbelscan.so
endef

define VDR_PLUGIN_WIRBELSCAN_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D) \
		$(VDR_PLUGIN_WIRBELSCAN_CXXFLAGS) \
		LIBDIR=$(TARGET_DIR)/usr/lib/vdr \
		VDRDIR=$(TARGET_DIR)/usr/share \
		i18n
endef

$(eval $(generic-package))
