################################################################################
#
# kodi-audiodecoder-vgmstream
#
################################################################################

KODI_AUDIODECODER_VGMSTREAM_VERSION = 3.0.0-Matrix
KODI_AUDIODECODER_VGMSTREAM_SITE = $(call github,xbmc,audiodecoder.vgmstream,$(KODI_AUDIODECODER_VGMSTREAM_VERSION))
KODI_AUDIODECODER_VGMSTREAM_LICENSE = GPL-2.0+
KODI_AUDIODECODER_VGMSTREAM_LICENSE_FILES = LICENSE.md
KODI_AUDIODECODER_VGMSTREAM_DEPENDENCIES = kodi

ifeq ($(BR2_TOOLCHAIN_GCC_AT_LEAST_5),)
KODI_AUDIODECODER_VGMSTREAM_C_FLAGS += -std=gnu99
endif

KODI_AUDIODECODER_VGMSTREAM_CONF_OPTS += \
	-DCMAKE_C_FLAGS="$(TARGET_CFLAGS) $(KODI_AUDIODECODER_VGMSTREAM_C_FLAGS)"

$(eval $(cmake-package))
