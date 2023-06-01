define HCONV_EXTRACT_CMDS
	cp package/fli4l/hconv/hconv.c $(@D)/
endef

define HCONV_BUILD_CMDS
	+$(MAKE) CC=$(TARGET_CC) LD=$(TARGET_LD) CFLAGS="$(TARGET_CFLAGS)" \
		-C $(@D) hconv
endef

define HCONV_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/hconv $(TARGET_DIR)/usr/bin/hconv
endef

$(eval $(generic-package))
