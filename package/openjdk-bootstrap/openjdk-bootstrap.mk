################################################################################
#
# openjdk-bootstrap
#
################################################################################

OPENJDK_BOOTSTRAP_VERSION = 1.8.0.141
OPENJDK_BOOTSTRAP_SITE = http://anduin.linuxfromscratch.org/BLFS/OpenJDK/OpenJDK-$(OPENJDK_BOOTSTRAP_VERSION)
ifeq ($(BR2_HOSTARCH),x86)
OPENJDK_BOOTSTRAP_SOURCE = OpenJDK-$(OPENJDK_BOOTSTRAP_VERSION)-i686-bin.tar.xz
else
OPENJDK_BOOTSTRAP_SOURCE = OpenJDK-$(OPENJDK_BOOTSTRAP_VERSION)-x86_64-bin.tar.xz
endif
OPENJDK_BOOTSTRAP_LICENSE = GPL-2.0+ with exception
OPENJDK_BOOTSTRAP_LICENSE_FILES = LICENSE

# Also provided to other packages
OPENJDK_BOOTSTRAP_DIR = $(HOST_DIR)/opt/jdk-bootstrap/

define HOST_OPENJDK_BOOTSTRAP_INSTALL_CMDS
	mkdir -p $(OPENJDK_BOOTSTRAP_DIR)
	cp -r $(@D)/* $(OPENJDK_BOOTSTRAP_DIR)
endef

$(eval $(host-generic-package))
