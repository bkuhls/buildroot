################################################################################
#
# kodi-audiodecoder-nosefart
#
################################################################################

KODI_AUDIODECODER_NOSEFART_VERSION = 780cae1e91152c2303db8b57ea0ee45a5184cb42
KODI_AUDIODECODER_NOSEFART_SITE = $(call github,xbmc,audiodecoder.nosefart,$(KODI_AUDIODECODER_NOSEFART_VERSION))
KODI_AUDIODECODER_NOSEFART_LICENSE = GPL-2.0+
KODI_AUDIODECODER_NOSEFART_LICENSE_FILES = LICENSE.md
KODI_AUDIODECODER_NOSEFART_DEPENDENCIES = kodi

$(eval $(cmake-package))
