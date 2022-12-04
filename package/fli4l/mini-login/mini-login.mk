ifeq ($(BR2_PACKAGE_LIBXCRYPT),y)
MINI_LOGIN_DEPENDENCIES += libxcrypt
endif

define MINI_LOGIN_EXTRACT_CMDS
	cp package/fli4l/mini-login/mini-login.c $(@D)/
endef

define MINI_LOGIN_BUILD_CMDS
	+$(MAKE) CC=$(TARGET_CC) LD=$(TARGET_LD) CFLAGS="$(TARGET_CFLAGS) -D_GNU_SOURCE" \
		LDLIBS="-lcrypt" -C $(@D) mini-login
endef

define MINI_LOGIN_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/mini-login $(TARGET_DIR)/usr/local/bin/mini-login
endef

$(eval $(generic-package))
