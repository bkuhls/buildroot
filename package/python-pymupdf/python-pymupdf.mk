################################################################################
#
# python-pymupdf
#
################################################################################

# python-pymupdf's version must be compatible with mupdf's version
PYTHON_PYMUPDF_VERSION = 1.28.2
PYTHON_PYMUPDF_SOURCE = pymupdf-$(PYTHON_PYMUPDF_VERSION).tar.gz
PYTHON_PYMUPDF_SITE = https://files.pythonhosted.org/packages/a3/fb/b6761fa2d5266f2cdb24c3b91f4023070ab7848381417678e7a289a1d52a
PYTHON_PYMUPDF_SETUP_TYPE = pep517
PYTHON_PYMUPDF_LICENSE = AGPL-3.0+
PYTHON_PYMUPDF_LICENSE_FILES = COPYING
# No license file included in pip, but it's present on github
PYTHON_PYMUPDF_DEPENDENCIES = freetype host-python-pipcl host-swig mupdf
PYTHON_PYMUPDF_BUILD_OPTS = --skip-dependency-check

# LD needs to be overriden because pipcl expects it to be a linker
# driver (e.g. gcc) and not an actual linker.
PYTHON_PYMUPDF_ENV = \
	LD=$(TARGET_CC) \
	PYMUPDF_INCLUDES="$(STAGING_DIR)/usr/include/freetype2:$(STAGING_DIR)/usr/include" \
	PYMUPDF_MUPDF_LIB="$(STAGING_DIR)/usr/lib" \
	PIPCL_PYTHON_CONFIG="$(STAGING_DIR)/usr/bin/python3-config" \
	PYMUPDF_SETUP_MUPDF_BUILD=

$(eval $(python-package))
