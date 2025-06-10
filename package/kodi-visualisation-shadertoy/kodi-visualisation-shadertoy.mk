################################################################################
#
# kodi-visualisation-shadertoy
#
################################################################################

KODI_VISUALISATION_SHADERTOY_VERSION = 5089433f208c01e67e88e7bfa243b004e8136e35
KODI_VISUALISATION_SHADERTOY_SITE = $(call github,xbmc,visualization.shadertoy,$(KODI_VISUALISATION_SHADERTOY_VERSION))
KODI_VISUALISATION_SHADERTOY_LICENSE = GPL-2.0+
KODI_VISUALISATION_SHADERTOY_LICENSE_FILES = LICENSE.md
KODI_VISUALISATION_SHADERTOY_DEPENDENCIES = glm jsoncpp kodi

$(eval $(cmake-package))
