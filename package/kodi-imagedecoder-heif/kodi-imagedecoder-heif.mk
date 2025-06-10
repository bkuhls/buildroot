################################################################################
#
# kodi-imagedecoder-heif
#
################################################################################

KODI_IMAGEDECODER_HEIF_VERSION = edc3d318a60fd12f383232a347db8f91f44df99e
KODI_IMAGEDECODER_HEIF_SITE = $(call github,xbmc,imagedecoder.heif,$(KODI_IMAGEDECODER_HEIF_VERSION))
KODI_IMAGEDECODER_HEIF_LICENSE = GPL-2.0+
KODI_IMAGEDECODER_HEIF_LICENSE_FILES = LICENSE.md
KODI_IMAGEDECODER_HEIF_DEPENDENCIES = kodi libde265 libheif tinyxml2

$(eval $(cmake-package))
