################################################################################
#
# Spidermonkey (1.8.5)
#
################################################################################

SPIDERMONKEY185_VERSION = 1.0.0
SPIDERMONKEY185_SITE = http://ftp.mozilla.org/pub/js
SPIDERMONKEY185_SOURCE = js185-$(SPIDERMONKEY185_VERSION).tar.gz
SPIDERMONKEY185_SUBDIR = js/src
SPIDERMONKEY185_LICENSE = MPL-1.1 or GPL-2.0+ or LGPL-2.1+
SPIDERMONKEY185_INSTALL_STAGING = YES
SPIDERMONKEY185_DEPENDENCIES = host-python3 host-perl

# To detect endianess a host binary is built from jscpucfg.cpp which is
# not cross-compile friendly, so we force endianess.
# HOST_CFLAGS are forced to avoid problems when detecting host gcc
# because configure would pass TARGET_CFLAGS to the host gcc.
SPIDERMONKEY185_CONF_ENV = \
	CFLAGS="$(TARGET_CFLAGS) -std=gnu17" \
	HOST_CFLAGS="$(HOST_CFLAGS)" \
	HOST_CXXFLAGS="$(HOST_CXXFLAGS) -DFORCE_$(BR2_ENDIAN)_ENDIAN" \
	$(if $(BR2_powerpc)$(BR2_x86_64),ac_cv_va_val_copy=no)

# Mozilla mixes up target, host and build.  See the comment in configure.in
# around line 360.  Also, nanojit fails to build on sparc64 with
# #error "unknown nanojit architecture", so disable the JIT.
# Disable JIT for armv4 because this CPU does not support the asm code
# used in spidermonkey.
SPIDERMONKEY185_CONF_OPTS = \
	--target=$(GNU_TARGET_NAME) \
	--build=$(GNU_TARGET_NAME) \
	--host=$(GNU_HOST_NAME) \
	$(if $(BR2_ARM_CPU_ARMV4),--disable-methodjit) \
	$(if $(BR2_sparc64),--disable-tracejit)

$(eval $(autotools-package))
