################################################################################
#
# libplacebo
#
################################################################################

LIBPLACEBO_VERSION = 6.338.2
LIBPLACEBO_SOURCE = libplacebo-v$(LIBPLACEBO_VERSION).tar.gz
LIBPLACEBO_SITE = https://code.videolan.org/videolan/libplacebo/-/archive/v$(LIBPLACEBO_VERSION)
LIBPLACEBO_LICENSE = LGPL-2.1+
LIBPLACEBO_LICENSE_FILES = LICENSE
LIBPLACEBO_INSTALL_STAGING = YES

$(eval $(meson-package))
