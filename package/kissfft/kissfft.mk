################################################################################
#
# kissfft
#
################################################################################

KISSFFT_VERSION = 131.1.0
KISSFFT_SITE = $(call github,mborgerding,kissfft,$(KISSFFT_VERSION))
KISSFFT_LICENSE = BSD-3-Clause
KISSFFT_LICENSE_FILES = COPYING LICENSES/BSD-3-Clause
KISSFFT_INSTALL_STAGING = YES
KISSFFT_CONF_OPTS = -DKISSFFT_TEST=OFF -DKISSFFT_TOOLS=OFF

ifeq ($(BR2_STATIC_LIBS),y)
KISSFFT_CONF_OPTS += -DBUILD_SHARED_LIBS=OFF -DKISSFFT_STATIC=ON
else
# cannot build static and shared together
KISSFFT_CONF_OPTS += -DBUILD_SHARED_LIBS=ON -DKISSFFT_STATIC=OFF
endif

$(eval $(cmake-package))
