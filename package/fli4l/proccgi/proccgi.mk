PROCCGI_SOURCE = proccgi.c
PROCCGI_SITE   = http://www.fpx.de/fp/Software
PROCCGI_EXTRACT_CMDS = cp -dp $(PROCCGI_DL_DIR)/$(PROCCGI_SOURCE) $(PROCCGI_DIR)

PROCCGI_LICENSE = GPL
PROCCGI_LICENSE_FILES =

define PROCCGI_BUILD_CMDS
	+$(MAKE) CC=$(TARGET_CC) LD=$(TARGET_LD) CFLAGS="$(TARGET_CFLAGS)" \
		-C $(@D) proccgi
endef

define PROCCGI_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/proccgi $(TARGET_DIR)/usr/local/bin/proccgi
endef

$(eval $(generic-package))
