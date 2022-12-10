define NETCALC_FLI4L_EXTRACT_CMDS
	cp package/fli4l/netcalc-fli4l/netcalc.c $(@D)/
endef

define NETCALC_FLI4L_BUILD_CMDS
	+$(MAKE) CC=$(TARGET_CC) LD=$(TARGET_LD) CFLAGS="$(TARGET_CFLAGS)" \
		-C $(@D) netcalc
endef

define NETCALC_FLI4L_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/netcalc $(TARGET_DIR)/usr/bin/netcalc
endef

$(eval $(generic-package))
