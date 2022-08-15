################################################################################
#
# p2pool
#
################################################################################

P2POOL_VERSION = v2.7
P2POOL_SITE_METHOD = git
P2POOL_SITE = https://github.com/SChernykh/p2pool.git
P2POOL_GIT_SUBMODULES = YES
P2POOL_LICENSE = GPL-3.0+
P2POOL_LICENSE_FILES = LICENSE
P2POOL_DEPENDENCIES = libcurl libuv zeromq

# prevent compiler warnings being treated as errors
define P2POOL_REMOVE_WARNING_FLAGS
	$(SED) '/set(WARNING_FLAGS/d' $(@D)/cmake/flags.cmake
endef
P2POOL_POST_PATCH_HOOKS += P2POOL_REMOVE_WARNING_FLAGS

# Upstream provides no installation rule:
#   *** No rule to make target 'install/fast'.  Stop.
define P2POOL_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/p2pool $(TARGET_DIR)/usr/bin/p2pool
endef

$(eval $(cmake-package))
