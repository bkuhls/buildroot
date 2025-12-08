################################################################################
#
# kodi-pvr-mythtv
#
################################################################################

KODI_PVR_MYTHTV_VERSION = b113c2b930a709d92cbce13cd1f34819aa3af5b4
KODI_PVR_MYTHTV_SITE = $(call github,janbar,pvr.mythtv,$(KODI_PVR_MYTHTV_VERSION))
KODI_PVR_MYTHTV_LICENSE = GPL-2.0+
KODI_PVR_MYTHTV_LICENSE_FILES = LICENSE.md
KODI_PVR_MYTHTV_DEPENDENCIES = kodi

$(eval $(cmake-package))
