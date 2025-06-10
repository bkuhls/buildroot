################################################################################
#
# kodi-audiodecoder-modplug
#
################################################################################

KODI_AUDIODECODER_MODPLUG_VERSION = cdfdb6311fd6805a18dcf10a021afa7b2b7036ee
KODI_AUDIODECODER_MODPLUG_SITE = $(call github,xbmc,audiodecoder.modplug,$(KODI_AUDIODECODER_MODPLUG_VERSION))
KODI_AUDIODECODER_MODPLUG_LICENSE = GPL-2.0+
KODI_AUDIODECODER_MODPLUG_LICENSE_FILES = LICENSE.md
KODI_AUDIODECODER_MODPLUG_DEPENDENCIES = kodi libmodplug

$(eval $(cmake-package))
