################################################################################
#
# python-pycryptodome
#
################################################################################

PYTHON_PYCRYPTODOME_VERSION = 3.4.6
PYTHON_PYCRYPTODOME_SOURCE = pycryptodome-$(PYTHON_PYCRYPTODOME_VERSION).tar.gz
PYTHON_PYCRYPTODOME_SITE = https://pypi.python.org/packages/b7/fb/5ea74c6dfe6c4937f2ca2e609155c0d97bdf00f8aa306deb25c8f547fdb8
PYTHON_PYCRYPTODOME_SETUP_TYPE = setuptools
PYTHON_PYCRYPTODOME_LICENSE = BSD, Public Domain

$(eval $(python-package))
