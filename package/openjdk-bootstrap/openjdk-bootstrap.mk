################################################################################
#
# openjdk-bootstrap
#
################################################################################

OPENJDK_BOOTSTRAP_VERSION = 14.0.1
ifeq ($(BR2_HOSTARCH),x86)
OPENJDK_BOOTSTRAP_SITE = http://anduin.linuxfromscratch.org/BLFS/OpenJDK/OpenJDK-$(OPENJDK_BOOTSTRAP_VERSION)
OPENJDK_BOOTSTRAP_SOURCE = OpenJDK-$(OPENJDK_BOOTSTRAP_VERSION)+7-i686-bin.tar.xz
else
OPENJDK_BOOTSTRAP_SITE = https://download.java.net/java/GA/jdk$(OPENJDK_BOOTSTRAP_VERSION)/664493ef4a6946b186ff29eb326336a2/7/GPL
OPENJDK_BOOTSTRAP_SOURCE = openjdk-$(OPENJDK_BOOTSTRAP_VERSION)_linux-x64_bin.tar.gz
endif
OPENJDK_BOOTSTRAP_LICENSE = GPL-2.0+ with exception
OPENJDK_BOOTSTRAP_LICENSE_FILES = legal/java.base/LICENSE

# Also provided to other packages
OPENJDK_BOOTSTRAP_DIR = $(HOST_DIR)/opt/jdk-bootstrap/

define HOST_OPENJDK_BOOTSTRAP_INSTALL_CMDS
	mkdir -p $(OPENJDK_BOOTSTRAP_DIR)
	cp -r $(@D)/* $(OPENJDK_BOOTSTRAP_DIR)
endef

$(eval $(host-generic-package))
