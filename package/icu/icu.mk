################################################################################
#
# icu
#
################################################################################

# Git tags (and therefore versions on release-monitoring.org) use the
# XX-Y format, but the tarballs are named XX_Y and the containing
# directories XX.Y.
ICU_VERSION_MAJOR = 77
ICU_VERSION = $(ICU_VERSION_MAJOR)-1
ICU_SOURCE = icu4c-$(subst -,_,$(ICU_VERSION))-src.tgz
ICU_SITE = \
	https://github.com/unicode-org/icu/releases/download/release-$(ICU_VERSION)
ICU_LICENSE = ICU License
ICU_LICENSE_FILES = LICENSE
ICU_CPE_ID_VENDOR = icu-project
ICU_CPE_ID_PRODUCT = international_components_for_unicode
ICU_CPE_ID_VERSION = $(subst -,.,$(ICU_VERSION))

ICU_DEPENDENCIES = host-icu
ICU_INSTALL_STAGING = YES
ICU_CONFIG_SCRIPTS = icu-config
ICU_CONF_OPTS = \
	--with-cross-build=$(HOST_ICU_DIR)/source \
	--disable-samples \
	--disable-tests

# the icu build process breaks if the TARGET environment variable is
# non-empty
ICU_CONF_ENV += TARGET=""
ICU_MAKE_ENV += TARGET=""
HOST_ICU_CONF_ENV += TARGET=""
HOST_ICU_MAKE_ENV += TARGET=""

# When available, icu prefers to use C++11 atomics, which rely on the
# __atomic builtins. On certain architectures, this requires linking
# with libatomic starting from gcc 4.8.
ifeq ($(BR2_TOOLCHAIN_HAS_LIBATOMIC),y)
ICU_CONF_ENV += LIBS="-latomic"
endif

# strtod_l() is not supported by musl; also xlocale.h is missing
ifeq ($(BR2_TOOLCHAIN_USES_MUSL),y)
ICU_CONF_ENV += ac_cv_func_strtod_l=no
endif

HOST_ICU_CONF_OPTS = \
	--disable-samples \
	--disable-tests \
	--disable-extras \
	--disable-icuio \
	--disable-layout
ICU_SUBDIR = source
HOST_ICU_SUBDIR = source

# ICU build scripting adds paths to LD_LIBRARY_PATH using
#   LD_LIBRARY_PATH="custom-path:${LD_LIBRARY_PATH}"
# which, if LD_LIBRARY_PATH was empty, causes the last search directory
# to be the working directory, causing the build to try to load target
# libraries, possibly crashing the build due to ABI mismatches.
# Workaround by ensuring LD_LIBRARY_PATH is never empty.
# https://unicode-org.atlassian.net/browse/ICU-21417
ifeq ($(LD_LIBRARY_PATH),)
ICU_MAKE_ENV += LD_LIBRARY_PATH=/dev/null
endif

ICU_CUSTOM_DATA_PATH = $(call qstrip,$(BR2_PACKAGE_ICU_CUSTOM_DATA_PATH))

ifneq ($(ICU_CUSTOM_DATA_PATH),)
define ICU_COPY_CUSTOM_DATA
	cp $(ICU_CUSTOM_DATA_PATH) $(@D)/source/data/in/
endef
ICU_POST_PATCH_HOOKS += ICU_COPY_CUSTOM_DATA
endif

ICU_DATA_FILTER_FILE = $(call qstrip,$(BR2_PACKAGE_ICU_DATA_FILTER_FILE))

ifneq ($(ICU_DATA_FILTER_FILE),)
HOST_ICU_DATA_SOURCE = $(subst src.tgz,data.zip,$(ICU_SOURCE))
HOST_ICU_EXTRA_DOWNLOADS += $(HOST_ICU_SITE)/$(HOST_ICU_DATA_SOURCE)

define HOST_ICU_EXTRACT_DATA
	rm -rf $(@D)/$(HOST_ICU_SUBDIR)/data
	$(UNZIP) $(ICU_DL_DIR)/$(HOST_ICU_DATA_SOURCE) -d $(@D)/$(HOST_ICU_SUBDIR)
endef
HOST_ICU_POST_EXTRACT_HOOKS += HOST_ICU_EXTRACT_DATA

HOST_ICU_CONF_ENV = ICU_DATA_FILTER_FILE=$(ICU_DATA_FILTER_FILE)
HOST_ICU_CONF_OPTS += --with-data-packaging=archive

define ICU_COPY_CUSTOM_DATA
	$(INSTALL) -D -m 644 $(HOST_ICU_DIR)/$(HOST_ICU_SUBDIR)/data/out/icudt$(ICU_VERSION_MAJOR)l.dat $(@D)/$(ICU_SUBDIR)/data/in/
endef
ICU_PRE_CONFIGURE_HOOKS += ICU_COPY_CUSTOM_DATA
endif

define ICU_REMOVE_DEV_FILES
	rm -f $(addprefix $(TARGET_DIR)/usr/bin/,derb genbrk gencfu gencnval gendict genrb icuinfo makeconv uconv)
	rm -f $(addprefix $(TARGET_DIR)/usr/sbin/,genccode gencmn gennorm2 gensprep icupkg)
	rm -rf $(TARGET_DIR)/usr/share/icu
endef
ICU_POST_INSTALL_TARGET_HOOKS += ICU_REMOVE_DEV_FILES

$(eval $(autotools-package))
$(eval $(host-autotools-package))
