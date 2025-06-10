################################################################################
#
# kodi-vfs-rar
#
################################################################################

KODI_VFS_RAR_VERSION = 6edc7c11e3594d393d462da2ed4b8fb9740bbb92
KODI_VFS_RAR_SITE = $(call github,xbmc,vfs.rar,$(KODI_VFS_RAR_VERSION))
KODI_VFS_RAR_LICENSE = unrar, GPL-2.0+
KODI_VFS_RAR_LICENSE_FILES = lib/UnrarXLib/license.txt LICENSE.md
KODI_VFS_RAR_DEPENDENCIES = kodi tinyxml

$(eval $(cmake-package))
