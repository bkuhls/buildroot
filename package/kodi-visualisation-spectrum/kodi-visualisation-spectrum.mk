################################################################################
#
# kodi-visualisation-spectrum
#
################################################################################

KODI_VISUALISATION_SPECTRUM_VERSION = 960a3595ca4b834d73b6990fe017a25fdccd32ba
KODI_VISUALISATION_SPECTRUM_SITE = $(call github,xbmc,visualization.spectrum,$(KODI_VISUALISATION_SPECTRUM_VERSION))
KODI_VISUALISATION_SPECTRUM_LICENSE = GPL-2.0+
KODI_VISUALISATION_SPECTRUM_LICENSE_FILES = LICENSE.md
KODI_VISUALISATION_SPECTRUM_DEPENDENCIES = glm kodi

$(eval $(cmake-package))
