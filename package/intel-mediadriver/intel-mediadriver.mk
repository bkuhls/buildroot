################################################################################
#
# intel-mediadriver
#
################################################################################

# based on https://software.intel.com/en-us/articles/build-and-debug-open-source-media-stack

INTEL_MEDIADRIVER_VERSION = 29d5f8fc118476dee84f3f784443badc819a3241
INTEL_MEDIADRIVER_SITE = $(call github,intel,media-driver,$(INTEL_MEDIADRIVER_VERSION))
INTEL_MEDIADRIVER_LICENSE = MIT, BSD-3-Clause
INTEL_MEDIADRIVER_LICENSE_FILES = LICENSE.md

INTEL_MEDIADRIVER_DEPENDENCIES = \
	intel-gmmlib \
	libpciaccess \
	libva \
	mesa3d

ifeq ($(BR2_PACKAGE_XLIB_LIBX11),y)
INTEL_MEDIADRIVER_DEPENDENCIES += xlib_libX11
endif

INTEL_MEDIADRIVER_SUPPORTS_IN_SOURCE_BUILD = NO

INTEL_MEDIADRIVER_CONF_OPTS = \
	-DINSTALL_DRIVER_SYSCONF=OFF \
	-DMEDIA_BUILD_FATAL_WARNINGS=OFF \
	-DMEDIA_RUN_TEST_SUITE=OFF

$(eval $(cmake-package))
