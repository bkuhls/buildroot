################################################################################
#
# qt5webkit
#
################################################################################

QT5WEBKIT_VERSION = 5.212.0-alpha4
QT5WEBKIT_SITE = https://github.com/qtwebkit/qtwebkit/releases/download/qtwebkit-$(QT5WEBKIT_VERSION)
QT5WEBKIT_SOURCE = qtwebkit-$(QT5WEBKIT_VERSION).tar.xz
QT5WEBKIT_DEPENDENCIES = \
	host-bison host-flex host-gperf host-python3 host-ruby gstreamer1 \
	gst1-plugins-base icu leveldb jpeg libpng libxml2 libxslt qt5location \
	openssl qt5sensors qt5webchannel sqlite webp woff2
QT5WEBKIT_INSTALL_STAGING = YES

QT5WEBKIT_LICENSE_FILES = Source/WebCore/LICENSE-LGPL-2 Source/WebCore/LICENSE-LGPL-2.1

QT5WEBKIT_LICENSE = LGPL-2.1+, BSD-3-Clause, BSD-2-Clause
# Source files contain references to LGPL_EXCEPTION.txt but it is not included
# in the archive.
QT5WEBKIT_LICENSE_FILES += LICENSE.LGPLv21

ifeq ($(BR2_MIPS_CPU_MIPS32R6),y)
QT5WEBKIT_CONF_OPTS += -DENABLE_JIT=OFF
endif

# To prevent OOM when building several webkit packages in parallel we
# force a build order:
# 1. webkitgtk
# 2. wpewebkit
# 3. qt5webkit
# 4. qt5webengine
ifeq ($(BR2_PER_PACKAGE_DIRECTORIES),y)
ifeq ($(BR2_PACKAGE_WEBKITGTK),y)
QT5WEBKIT_DEPENDENCIES += webkitgtk
endif
ifeq ($(BR2_PACKAGE_WPEWEBKIT),y)
QT5WEBKIT_DEPENDENCIES += wpewebkit
endif
endif

ifeq ($(BR2_PACKAGE_QT5BASE_OPENGL),y)
QT5WEBKIT_CONF_OPTS += \
	-DENABLE_OPENGL=ON \
	-DENABLE_WEBKIT2=ON
else
QT5WEBKIT_CONF_OPTS += \
	-DENABLE_OPENGL=OFF \
	-DENABLE_WEBKIT2=OFF
endif

ifeq ($(BR2_PACKAGE_QT5BASE_XCB),y)
QT5WEBKIT_DEPENDENCIES += xlib_libXcomposite xlib_libXext xlib_libXrender
endif

ifeq ($(BR2_PACKAGE_QT5DECLARATIVE),y)
QT5WEBKIT_DEPENDENCIES += qt5declarative
endif

ifeq ($(BR2_PACKAGE_LIBEXECINFO),y)
QT5WEBKIT_DEPENDENCIES += libexecinfo
endif

ifeq ($(BR2_TOOLCHAIN_USES_MUSL),y)
QT5WEBKIT_CONF_OPTS += -DENABLE_SAMPLING_PROFILER=OFF
endif

QT5WEBKIT_CONF_OPTS += \
	-DKDE_INSTALL_USE_QT_SYS_PATHS=ON \
	-DENABLE_TOOLS=OFF \
	-DPORT=Qt \
	-DPYTHON_EXECUTABLE=$(HOST_DIR)/bin/python3 \
	-DSHARED_CORE=ON \
	-DUSE_LIBHYPHEN=OFF

# Remove the DESTDIR from the install command install so that the .pri files in
# the correct location: $(HOST_DIR)/mkspecs/modules instead of
# $(STAGING_DIR)$(HOST_DIR)/usr/mkspecs/modules.
define QT5WEBKIT_INSTALL_STAGING_CMDS
	$(TARGET_MAKE_ENV) $(QT5WEBKIT_BUILD_ENV) \
		$(BR2_CMAKE) --install $(QT5WEBKIT_BUILDDIR) \
		--prefix $(STAGING_DIR)/usr
endef

# Remove the DESTDIR from the install command install so that the .pri files in
# the correct location: $(HOST_DIR)/mkspecs/modules instead of
# $(TARGET_DIR)$(HOST_DIR)/usr/mkspecs/modules.
define QT5WEBKIT_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(QT5WEBKIT_BUILD_ENV) \
		$(BR2_CMAKE) --install $(QT5WEBKIT_BUILDDIR) \
		--prefix $(TARGET_DIR)/usr
endef

$(eval $(cmake-package))
