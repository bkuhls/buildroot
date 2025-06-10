################################################################################
#
# kodi-pvr-mythtv
#
################################################################################

KODI_PVR_MYTHTV_VERSION = 373c821d01d1cd2fd7be77123fd7fe2adab4c389
KODI_PVR_MYTHTV_SITE = $(call github,janbar,pvr.mythtv,$(KODI_PVR_MYTHTV_VERSION))
KODI_PVR_MYTHTV_LICENSE = GPL-2.0+
KODI_PVR_MYTHTV_LICENSE_FILES = LICENSE.md
KODI_PVR_MYTHTV_DEPENDENCIES = kodi

$(eval $(cmake-package))
