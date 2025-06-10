################################################################################
#
# kodi-imagedecoder-raw
#
################################################################################

KODI_IMAGEDECODER_RAW_VERSION = 0a2961c80be82fc6edc64c4edd9f540762a182e8
KODI_IMAGEDECODER_RAW_SITE = $(call github,xbmc,imagedecoder.raw,$(KODI_IMAGEDECODER_RAW_VERSION))
KODI_IMAGEDECODER_RAW_LICENSE = GPL-2.0+
KODI_IMAGEDECODER_RAW_LICENSE_FILES = LICENSE.md
KODI_IMAGEDECODER_RAW_DEPENDENCIES = kodi jpeg lcms2 libraw

$(eval $(cmake-package))
