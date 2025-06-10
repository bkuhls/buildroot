################################################################################
#
# kodi-audiodecoder-timidity
#
################################################################################

KODI_AUDIODECODER_TIMIDITY_VERSION = 5716cb3caa6fff2ca000bfe6d4bbefa02a9c95e6
KODI_AUDIODECODER_TIMIDITY_SITE = $(call github,xbmc,audiodecoder.timidity,$(KODI_AUDIODECODER_TIMIDITY_VERSION))
KODI_AUDIODECODER_TIMIDITY_LICENSE = GPL-2.0+
KODI_AUDIODECODER_TIMIDITY_LICENSE_FILES = LICENSE.md
KODI_AUDIODECODER_TIMIDITY_DEPENDENCIES = kodi

$(eval $(cmake-package))
