################################################################################
#
# vdr-plugin-wirbelscancontrol
#
################################################################################

VDR_PLUGIN_WIRBELSCANCONTROL_VERSION = 0.0.2
VDR_PLUGIN_WIRBELSCANCONTROL_SOURCE = vdr-wirbelscancontrol-$(VDR_PLUGIN_WIRBELSCANCONTROL_VERSION).tgz
VDR_PLUGIN_WIRBELSCANCONTROL_SITE = http://wirbel.htpc-forum.de/wirbelscancontrol
VDR_PLUGIN_WIRBELSCANCONTROL_LICENSE = GPLv2+
VDR_PLUGIN_WIRBELSCANCONTROL_LICENSE_FILES = COPYING
VDR_PLUGIN_WIRBELSCANCONTROL_DEPENDENCIES = vdr-plugin-wirbelscan

VDR_PLUGIN_WIRBELSCANCONTROL_CXXFLAGS = \
	CXXFLAGS="$(TARGET_CXXFLAGS) -fPIC"

define VDR_PLUGIN_WIRBELSCANCONTROL_BUILD_CMDS
	ln -sf $(VDR_PLUGIN_WIRBELSCAN_BUILDDIR)/wirbelscan_services.h $(@D)
	$(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D) \
		$(VDR_PLUGIN_WIRBELSCANCONTROL_CXXFLAGS) \
		LIBDIR=$(TARGET_DIR)/usr/lib/vdr \
		VDRDIR=$(STAGING_DIR)/usr/include/vdr \
		libvdr-wirbelscancontrol.so
endef

define VDR_PLUGIN_WIRBELSCANCONTROL_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D) \
		$(VDR_PLUGIN_WIRBELSCANCONTROL_CXXFLAGS) \
		LIBDIR=$(TARGET_DIR)/usr/lib/vdr \
		VDRDIR=$(TARGET_DIR)/usr/share \
		i18n
endef

$(eval $(generic-package))
