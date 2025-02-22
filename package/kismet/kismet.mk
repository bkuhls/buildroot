################################################################################
#
# kismet
#
################################################################################

KISMET_VERSION = 52bcb902e36b2df1562cfbe644b113362248d029
KISMET_SITE = $(call github,kismetwireless,kismet,$(KISMET_VERSION))
KISMET_DEPENDENCIES = \
	host-pkgconf \
	libpcap \
	$(if $(BR2_PACKAGE_LIBNL),libnl) \
	$(if $(BR2_PACKAGE_OPENSSL),openssl) \
	$(if $(BR2_PACKAGE_PROTOBUF),protobuf) \
	protobuf-c \
	sqlite \
	zlib
KISMET_LICENSE = GPL-2.0+
KISMET_LICENSE_FILES = LICENSE
KISMET_SELINUX_MODULES = kismet

KISMET_CONF_OPTS = --disable-debuglibs --disable-wifi-coconut

KISMET_CXXFLAGS = $(TARGET_CXXFLAGS)

ifeq ($(BR2_TOOLCHAIN_HAS_GCC_BUG_85180),y)
KISMET_CXXFLAGS += -O0
endif

KISMET_CONF_ENV += CXXFLAGS="$(KISMET_CXXFLAGS)"

ifeq ($(BR2_PACKAGE_LIBCAP),y)
KISMET_DEPENDENCIES += libcap
KISMET_CONF_OPTS += --enable-libcap
else
KISMET_CONF_OPTS += --disable-libcap
endif

ifeq ($(BR2_PACKAGE_LIBRTLSDR),y)
KISMET_DEPENDENCIES += librtlsdr
KISMET_CONF_OPTS += --enable-librtlsdr
else
KISMET_CONF_OPTS += --disable-librtlsdr
endif

ifeq ($(BR2_PACKAGE_LIBUSB),y)
KISMET_DEPENDENCIES += libusb
KISMET_CONF_OPTS += --enable-libusb
else
KISMET_CONF_OPTS += --disable-libusb
endif

ifeq ($(BR2_PACKAGE_LIBWEBSOCKETS),y)
KISMET_DEPENDENCIES += libwebsockets
KISMET_CONF_OPTS += --enable-libwebsockets
else
KISMET_CONF_OPTS += --disable-libwebsockets
endif

ifeq ($(BR2_PACKAGE_LM_SENSORS),y)
KISMET_DEPENDENCIES += lm-sensors
KISMET_CONF_OPTS += --enable-lmsensors
else
KISMET_CONF_OPTS += --disable-lmsensors
endif

ifeq ($(BR2_PACKAGE_MOSQUITTO),y)
KISMET_DEPENDENCIES += mosquitto
KISMET_CONF_OPTS += --enable-mosquitto
else
KISMET_CONF_OPTS += --disable-mosquitto
endif

ifeq ($(BR2_PACKAGE_PCRE2),y)
KISMET_DEPENDENCIES += pcre2
KISMET_CONF_OPTS += --enable-pcre --enable-require-pcre2
else ifeq ($(BR2_PACKAGE_PCRE),y)
KISMET_DEPENDENCIES += pcre
KISMET_CONF_OPTS += --enable-pcre --disable-require-pcre2
else
KISMET_CONF_OPTS += --disable-pcre
endif

ifeq ($(BR2_PACKAGE_KISMET_PYTHON_TOOLS),y)
KISMET_DEPENDENCIES += python3 host-python-setuptools
KISMET_CONF_OPTS += \
	--enable-python-tools \
	--with-python-interpreter=$(HOST_DIR)/bin/python$(PYTHON3_VERSION_MAJOR)
else
KISMET_CONF_OPTS += --disable-python-tools
endif

KISMET_INSTALL_TARGET_OPTS += \
	DESTDIR=$(TARGET_DIR) \
	INSTUSR=$(shell id -u) \
	INSTGRP=$(shell id -g) \
	SUIDGROUP=$(shell id -g)

ifeq ($(BR2_PACKAGE_KISMET_SERVER),y)
KISMET_CONF_OPTS += --disable-capture-tools-only
KISMET_INSTALL_TARGET_OPTS += install
else
KISMET_CONF_OPTS += --enable-capture-tools-only
KISMET_INSTALL_TARGET_OPTS += binsuidinstall
endif

$(eval $(autotools-package))
