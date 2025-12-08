################################################################################
#
# kodi-pvr-mythtv
#
################################################################################

KODI_PVR_MYTHTV_VERSION = 882524f25e1f90a8cba538950071a652ad48e8c3
KODI_PVR_MYTHTV_SITE = $(call github,janbar,pvr.mythtv,$(KODI_PVR_MYTHTV_VERSION))
KODI_PVR_MYTHTV_LICENSE = GPL-2.0+
KODI_PVR_MYTHTV_LICENSE_FILES = LICENSE.md
KODI_PVR_MYTHTV_DEPENDENCIES = kodi

$(eval $(cmake-package))
