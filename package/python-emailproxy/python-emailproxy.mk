################################################################################
#
# python-emailproxy
#
################################################################################

PYTHON_EMAILPROXY_VERSION = 2024.11.11
PYTHON_EMAILPROXY_SOURCE = emailproxy-$(PYTHON_EMAILPROXY_VERSION).tar.gz
PYTHON_EMAILPROXY_SITE = https://files.pythonhosted.org/packages/2b/49/386e675f2b0706083fdf02ad3666b98d0875a2e1cdcabff8df9fe641d2e3
PYTHON_EMAILPROXY_SETUP_TYPE = setuptools
PYTHON_EMAILPROXY_LICENSE = Apache-2.0
PYTHON_EMAILPROXY_LICENSE_FILES = LICENSE

$(eval $(python-package))
