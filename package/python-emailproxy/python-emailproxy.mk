################################################################################
#
# python-emailproxy
#
################################################################################

PYTHON_EMAILPROXY_VERSION = 2024.9.12
PYTHON_EMAILPROXY_SOURCE = emailproxy-$(PYTHON_EMAILPROXY_VERSION).tar.gz
PYTHON_EMAILPROXY_SITE = https://files.pythonhosted.org/packages/a5/aa/42e408ec6f9c9e06d451b26ae97e127f4644860336b28636bbf14845a35a
PYTHON_EMAILPROXY_DEPENDENCIES = host-python-cryptography host-python-pyasyncore
PYTHON_EMAILPROXY_SETUP_TYPE = setuptools
PYTHON_EMAILPROXY_LICENSE = Apache-2.0
PYTHON_EMAILPROXY_LICENSE_FILES = LICENSE

$(eval $(python-package))
