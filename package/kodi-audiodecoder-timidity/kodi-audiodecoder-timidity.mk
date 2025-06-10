################################################################################
#
# kodi-audiodecoder-timidity
#
################################################################################

KODI_AUDIODECODER_TIMIDITY_VERSION = e4c37d2f9490d65e81cb0d8fc6206f548e5f12b9
KODI_AUDIODECODER_TIMIDITY_SITE = $(call github,xbmc,audiodecoder.timidity,$(KODI_AUDIODECODER_TIMIDITY_VERSION))
KODI_AUDIODECODER_TIMIDITY_LICENSE = GPL-2.0+
KODI_AUDIODECODER_TIMIDITY_LICENSE_FILES = LICENSE.md
KODI_AUDIODECODER_TIMIDITY_DEPENDENCIES = kodi

$(eval $(cmake-package))
