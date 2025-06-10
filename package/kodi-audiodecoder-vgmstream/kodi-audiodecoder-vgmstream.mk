################################################################################
#
# kodi-audiodecoder-vgmstream
#
################################################################################

KODI_AUDIODECODER_VGMSTREAM_VERSION = c1c3a1283422a95240649def1dccf5ac5969f810
KODI_AUDIODECODER_VGMSTREAM_SITE = $(call github,xbmc,audiodecoder.vgmstream,$(KODI_AUDIODECODER_VGMSTREAM_VERSION))
KODI_AUDIODECODER_VGMSTREAM_LICENSE = GPL-2.0+
KODI_AUDIODECODER_VGMSTREAM_LICENSE_FILES = LICENSE.md
KODI_AUDIODECODER_VGMSTREAM_DEPENDENCIES = kodi

$(eval $(cmake-package))
