################################################################################
#
# mudpf
#
################################################################################

# python-pymupdf's version be compatible with mupdf's version
MUPDF_VERSION = 1.28.5
MUPDF_SOURCE = mupdf-$(MUPDF_VERSION)-source.tar.lz
MUPDF_SITE = https://mupdf.com/downloads/archive
MUPDF_LICENSE = AGPL-3.0+
MUPDF_LICENSE_FILES = COPYING
MUPDF_CPE_ID_VENDOR = artifex
MUPDF_INSTALL_STAGING = YES
MUPDF_DEPENDENCIES = \
	freetype \
	gumbo-parser \
	harfbuzz \
	host-pkgconf \
	jbig2dec jpeg \
	lcms2 openjpeg \
	zlib

# mupdf doesn't use CFLAGS and LIBS but XCFLAGS and XLIBS instead.
# with USE_SYSTEM_LIBS it will try to use system libraries instead of the bundled ones.
MUPDF_MAKE_ENV = $(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) \
	XCFLAGS="$(TARGET_CFLAGS)" \
	XLIBS="$(TARGET_LDFLAGS)" \
	USE_SYSTEM_LIBS=yes

MUPDF_MAKE_OPTS = \
	HAVE_OBJCOPY=no \
	prefix="/usr"

ifeq ($(BR2_STATIC_LIBS),y)
MUPDF_MAKE_OPTS += shared=no
else
MUPDF_MAKE_OPTS += shared=yes

ifeq ($(BR2_PACKAGE_MUPDF_PYTHON_BINDINGS),y)
MUPDF_DEPENDENCIES += \
	host-clang \
	host-python-pipcl \
	host-swig \
	python3

MUPDF_MAKE_ENV += \
	VENV_FLAG="" \
	MUPDF_NO_AUTOVENV=1 \
	MUPDF_SETUP_USE_CLANG_PYTHON=0 \
	MUPDF_LIBCLANG_ARGS="-I$(STAGING_DIR)/usr/include -I$(HOST_DIR)/lib/clang/$(firstword $(subst ., ,$(HOST_CLANG_VERSION)))/include" \
	PIPCL_PYTHON_CONFIG="$(STAGING_DIR)/usr/bin/python3-config"

# mupdf generates C++ code for its python bindings. As things can
# easily go wrong, increase the log level as much as we can so that we
# at least always see libclang's diagnostics:
MUPDF_MAKE_ENV += JLIB_log_levels="/:=-5"

define MUPDF_BUILD_PYTHON_BINDINGS
	$(MUPDF_MAKE_ENV) $(MAKE) -C $(@D) $(MUPDF_MAKE_OPTS) python
endef

define MUPDF_INSTALL_PYTHON_BINDINGS_TO_STAGING
	$(MUPDF_MAKE_ENV) $(MAKE) -C $(@D) $(MUPDF_MAKE_OPTS) \
		DESTDIR="$(STAGING_DIR)" pydir=/usr/lib/python$(PYTHON3_VERSION_MAJOR) install-shared-python \
		-o python -o c++ # skip rebuilding bindings
endef
define MUPDF_INSTALL_PYTHON_BINDINGS_TO_TARGET
	$(MUPDF_MAKE_ENV) $(MAKE) -C $(@D) $(MUPDF_MAKE_OPTS) \
		DESTDIR="$(TARGET_DIR)" pydir=/usr/lib/python$(PYTHON3_VERSION_MAJOR) install-shared-python \
		-o python -o c++ # skip rebuilding bindings
endef
MUPDF_POST_BUILD_HOOKS += MUPDF_BUILD_PYTHON_BINDINGS
MUPDF_POST_INSTALL_STAGING_HOOKS += MUPDF_INSTALL_PYTHON_BINDINGS_TO_STAGING
MUPDF_POST_INSTALL_TARGET_HOOKS += MUPDF_INSTALL_PYTHON_BINDINGS_TO_TARGET
endif
endif

ifeq ($(BR2_PACKAGE_XLIB_LIBX11)$(BR2_PACKAGE_XLIB_LIBXEXT),yy)
MUPDF_MAKE_OPTS += HAVE_X11=yes
MUPDF_DEPENDENCIES += xlib_libX11 xlib_libXext
else
MUPDF_MAKE_OPTS += HAVE_X11=no
endif

ifeq ($(BR2_PACKAGE_LIBFREEGLUT),y)
MUPDF_DEPENDENCIES += libfreeglut
else
MUPDF_MAKE_OPTS += HAVE_GLUT=no
endif

ifeq ($(BR2_PACKAGE_BROTLI),y)
MUPDF_DEPENDENCIES += brotli
else
MUPDF_MAKE_OPTS += brotli=no
endif

define MUPDF_BUILD_CMDS
	$(MUPDF_MAKE_ENV) $(MAKE) -C $(@D) $(MUPDF_MAKE_OPTS) all
endef

define MUPDF_INSTALL_STAGING_CMDS
	$(MUPDF_MAKE_ENV) $(MAKE) -C $(@D) $(MUPDF_MAKE_OPTS) \
		DESTDIR="$(STAGING_DIR)" install-libs
endef

define MUPDF_INSTALL_TARGET_CMDS
	$(MUPDF_MAKE_ENV) $(MAKE) -C $(@D) $(MUPDF_MAKE_OPTS) \
		DESTDIR="$(TARGET_DIR)" install-libs install-apps
endef

$(eval $(generic-package))
