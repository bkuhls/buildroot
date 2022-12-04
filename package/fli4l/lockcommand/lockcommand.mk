define LOCKCOMMAND_EXTRACT_CMDS
	cp package/fli4l/lockcommand/lockcommand.c $(@D)/
endef

define LOCKCOMMAND_BUILD_CMDS
	+$(MAKE) CC=$(TARGET_CC) LD=$(TARGET_LD) CFLAGS="$(TARGET_CFLAGS)" \
		-C $(@D) lockcommand
endef

define LOCKCOMMAND_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/lockcommand $(TARGET_DIR)/sbin/lockcommand
endef

$(eval $(generic-package))
