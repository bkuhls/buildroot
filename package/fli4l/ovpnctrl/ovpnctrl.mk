define OVPNCTRL_EXTRACT_CMDS
	cp package/fli4l/ovpnctrl/ovpnctrl.c $(@D)/
endef

define OVPNCTRL_BUILD_CMDS
	+$(MAKE) CC=$(TARGET_CC) LD=$(TARGET_LD) CFLAGS="$(TARGET_CFLAGS)" \
		-C $(@D) ovpnctrl
endef

define OVPNCTRL_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/ovpnctrl $(TARGET_DIR)/usr/bin/ovpnctrl
endef

$(eval $(generic-package))
