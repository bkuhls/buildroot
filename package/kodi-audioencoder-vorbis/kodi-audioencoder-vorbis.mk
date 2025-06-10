################################################################################
#
# kodi-audioencoder-vorbis
#
################################################################################

KODI_AUDIOENCODER_VORBIS_VERSION = d9e7d578252e4036ef7d96b04a04a9c093ef61be
KODI_AUDIOENCODER_VORBIS_SITE = $(call github,xbmc,audioencoder.vorbis,$(KODI_AUDIOENCODER_VORBIS_VERSION))
KODI_AUDIOENCODER_VORBIS_LICENSE = GPL-2.0+
KODI_AUDIOENCODER_VORBIS_LICENSE_FILES = LICENSE.md
KODI_AUDIOENCODER_VORBIS_DEPENDENCIES = kodi libogg libvorbis host-pkgconf

$(eval $(cmake-package))
