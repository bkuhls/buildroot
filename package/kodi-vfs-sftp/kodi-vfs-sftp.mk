################################################################################
#
# kodi-vfs-sftp
#
################################################################################

KODI_VFS_SFTP_VERSION = 4e81c754dd0281db8bde712f2241ec9ce9ea5aaf
KODI_VFS_SFTP_SITE = $(call github,xbmc,vfs.sftp,$(KODI_VFS_SFTP_VERSION))
KODI_VFS_SFTP_LICENSE = GPL-2.0+
KODI_VFS_SFTP_LICENSE_FILES = LICENSE.md
KODI_VFS_SFTP_DEPENDENCIES = kodi libssh openssl zlib

$(eval $(cmake-package))
