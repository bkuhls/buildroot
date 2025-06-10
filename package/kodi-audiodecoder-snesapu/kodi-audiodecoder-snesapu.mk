################################################################################
#
# kodi-audiodecoder-snesapu
#
################################################################################

KODI_AUDIODECODER_SNESAPU_VERSION = bf4c78fa2c192d45076ddd8d9b56db8c20d2e9c9
KODI_AUDIODECODER_SNESAPU_SITE = $(call github,xbmc,audiodecoder.snesapu,$(KODI_AUDIODECODER_SNESAPU_VERSION))
KODI_AUDIODECODER_SNESAPU_LICENSE = GPL-2.0+
KODI_AUDIODECODER_SNESAPU_LICENSE_FILES = LICENSE.md
KODI_AUDIODECODER_SNESAPU_DEPENDENCIES = kodi

$(eval $(cmake-package))
