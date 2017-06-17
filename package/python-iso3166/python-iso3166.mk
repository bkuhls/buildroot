################################################################################
#
# python-iso3166
#
################################################################################

PYTHON_ISO3166_VERSION = 0.8
PYTHON_ISO3166_SOURCE = iso3166-$(PYTHON_ISO3166_VERSION).tar.gz
PYTHON_ISO3166_SITE = https://pypi.python.org/packages/46/06/64145b8d6be8474db1f09f6b01a083921c11a4c979d029677c7e943d2433
PYTHON_ISO3166_SETUP_TYPE = setuptools
PYTHON_ISO3166_LICENSE = MIT
PYTHON_ISO3166_LICENSE_FILES = LICENSE.txt

$(eval $(python-package))
