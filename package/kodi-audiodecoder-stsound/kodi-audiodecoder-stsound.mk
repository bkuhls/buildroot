################################################################################
#
# kodi-audiodecoder-stsound
#
################################################################################

KODI_AUDIODECODER_STSOUND_VERSION = fdb0adb1e8e8be36be27cbc8a93a9940044d029e
KODI_AUDIODECODER_STSOUND_SITE = $(call github,xbmc,audiodecoder.stsound,$(KODI_AUDIODECODER_STSOUND_VERSION))
KODI_AUDIODECODER_STSOUND_LICENSE = GPL-2.0+
KODI_AUDIODECODER_STSOUND_LICENSE_FILES = LICENSE.md
KODI_AUDIODECODER_STSOUND_DEPENDENCIES = kodi

$(eval $(cmake-package))
