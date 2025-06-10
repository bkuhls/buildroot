################################################################################
#
# kodi-audiodecoder-sidplay
#
################################################################################

KODI_AUDIODECODER_SIDPLAY_VERSION = d37f4f3a64629b249153c5c7b4b294eff866285c
KODI_AUDIODECODER_SIDPLAY_SITE = $(call github,xbmc,audiodecoder.sidplay,$(KODI_AUDIODECODER_SIDPLAY_VERSION))
KODI_AUDIODECODER_SIDPLAY_LICENSE = GPL-2.0+
KODI_AUDIODECODER_SIDPLAY_LICENSE_FILES = LICENSE.md
KODI_AUDIODECODER_SIDPLAY_DEPENDENCIES = host-pkgconf kodi libsidplay2

$(eval $(cmake-package))
