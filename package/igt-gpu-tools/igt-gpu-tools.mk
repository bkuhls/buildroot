################################################################################
#
# igt-gpu-tools
#
################################################################################

IGT_GPU_TOOLS_VERSION = 1.26
IGT_GPU_TOOLS_SITE = https://gitlab.freedesktop.org/drm/igt-gpu-tools/-/archive/igt-gpu-tools-$(IGT_GPU_TOOLS_VERSION)
IGT_GPU_TOOLS_SOURCE = igt-gpu-tools-$(IGT_GPU_TOOLS_VERSION).tar.bz2
IGT_GPU_TOOLS_LICENSE = MIT
IGT_GPU_TOOLS_LICENSE_FILES = COPYING
IGT_GPU_TOOLS_INSTALL_STAGING = YES
IGT_GPU_TOOLS_DEPENDENCIES = \
	elfutils \
	host-pkgconf \
	cairo \
	kmod \
	libdrm \
	libglib2 \
	openssl \
	libpciaccess \
	pixman \
	procps-ng \
	zlib

IGT_GPU_TOOLS_CONF_OPTS += -Ddocs=disabled

ifeq ($(BR2_PACKAGE_LIBUNWIND),y)
IGT_GPU_TOOLS_CONF_OPTS += -Dlibunwind=enabled
IGT_GPU_TOOLS_DEPENDENCIES += libunwind
else
IGT_GPU_TOOLS_CONF_OPTS += -Dlibunwind=disabled
endif

$(eval $(meson-package))
