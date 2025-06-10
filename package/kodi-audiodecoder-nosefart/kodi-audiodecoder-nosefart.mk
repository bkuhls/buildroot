################################################################################
#
# kodi-audiodecoder-nosefart
#
################################################################################

KODI_AUDIODECODER_NOSEFART_VERSION = a95837c07812b29ae09724df9c476aa6ac0514c8
KODI_AUDIODECODER_NOSEFART_SITE = $(call github,xbmc,audiodecoder.nosefart,$(KODI_AUDIODECODER_NOSEFART_VERSION))
KODI_AUDIODECODER_NOSEFART_LICENSE = GPL-2.0+
KODI_AUDIODECODER_NOSEFART_LICENSE_FILES = LICENSE.md
KODI_AUDIODECODER_NOSEFART_DEPENDENCIES = kodi

$(eval $(cmake-package))
