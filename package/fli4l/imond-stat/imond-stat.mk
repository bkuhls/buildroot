################################################################################
#
# imond-stat
#
################################################################################

IMOND_STAT_LICENSE = Public Domain

define IMOND_STAT_EXTRACT_CMDS
	cp package/fli4l/imond-stat/imond-stat.c $(@D)/
endef

define IMOND_STAT_BUILD_CMDS
	(cd $(@D); $(TARGET_CC) -Wall -Os -s imond-stat.c -o imond-stat)
endef

define IMOND_STAT_INSTALL_TARGET_CMDS
	install -m 0755 -D $(@D)/imond-stat $(TARGET_DIR)/usr/local/bin/imond-stat
endef

$(eval $(generic-package))
