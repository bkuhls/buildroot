################################################################################
#
# imond
#
################################################################################

IMOND_LICENSE = Public Domain

ifeq ($(BR2_PACKAGE_LIBXCRYPT),y)
IMOND_DEPENDENCIES += libxcrypt
endif

define IMOND_EXTRACT_CMDS
	cp package/fli4l/imond/{*.c,*.h} $(@D)/
endef

define IMOND_BUILD_CMDS
	(cd $(@D); $(TARGET_CC) -Wall -Os -lcrypt -s md5.c imond.c -o imond)
endef

define IMOND_INSTALL_TARGET_CMDS
	install -m 0755 -D $(@D)/imond $(TARGET_DIR)/usr/local/bin/imond
endef

$(eval $(generic-package))
