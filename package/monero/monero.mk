################################################################################
#
# monero
#
################################################################################

MONERO_VERSION = 0.18.1.2
MONERO_SOURCE = monero-source-v$(MONERO_VERSION).tar.bz2
MONERO_SITE = https://downloads.getmonero.org/cli
MONERO_LICENSE = MIT
MONERO_LICENSE_FILES = LICENSE
MONERO_CONF_OPTS = \
	-DBUILD_DOCUMENTATION=OFF \
	-DCMAKE_CXX_FLAGS="$(MONERO_CXXFLAGS)" \
	-DCMAKE_BUILD_TYPE=None \
	-DMANUAL_SUBMODULES=ON \
	-DSTACK_TRACE=OFF \
	-DUSE_CCACHE=OFF \
	-DUSE_DEVICE_TREZOR=OFF
MONERO_DEPENDENCIES = \
	boost \
	czmq \
	libminiupnpc \
	libsodium \
	openssl \
	unbound

ifeq ($(BR2_aarch64_be)$(BR2_mips)$(BR2_mipsel)$(BR2_mips64el)$(BR2_sparc64),y)
MONERO_CONF_OPTS += -DNO_AES=ON
endif

MONERO_CXXFLAGS = $(TARGET_CXXFLAGS) -std=c++11

# Uses __atomic_load_8
ifeq ($(BR2_TOOLCHAIN_HAS_LIBATOMIC),y)
MONERO_CXXFLAGS += -latomic
endif

# relocation truncated to fit: R_MIPS_GOT16
ifeq ($(BR2_mipsel),y)
MONERO_CXXFLAGS += -mxgot
endif

ifeq ($(BR2_PACKAGE_HIDAPI),y)
MONERO_DEPENDENCIES += hidapi
endif

ifeq ($(BR2_PACKAGE_READLINE),y)
MONERO_CONF_OPTS += -DUSE_READLINE=ON
MONERO_DEPENDENCIES += readline
else
MONERO_CONF_OPTS += -DUSE_READLINE=OFF
endif

$(eval $(cmake-package))
