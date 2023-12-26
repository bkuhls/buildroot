define DECODE_CHUNKED_EXTRACT_CMDS
	cp package/fli4l/decode_chunked/decode_chunked.c $(@D)/
endef

define DECODE_CHUNKED_BUILD_CMDS
	+$(MAKE) CC=$(TARGET_CC) LD=$(TARGET_LD) CFLAGS="$(TARGET_CFLAGS)" \
		-C $(@D) decode_chunked
endef

define DECODE_CHUNKED_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/decode_chunked $(TARGET_DIR)/usr/local/bin/decode_chunked
endef

$(eval $(generic-package))
