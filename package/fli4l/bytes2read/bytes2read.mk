define BYTES2READ_EXTRACT_CMDS
	cp package/fli4l/bytes2read/bytes2read.c $(@D)/
endef

define BYTES2READ_BUILD_CMDS
	+$(MAKE) CC=$(TARGET_CC) LD=$(TARGET_LD) CFLAGS="$(TARGET_CFLAGS)" \
		-C $(@D) bytes2read
endef

define BYTES2READ_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/bytes2read $(TARGET_DIR)/usr/local/bin/bytes2read
endef

$(eval $(generic-package))
