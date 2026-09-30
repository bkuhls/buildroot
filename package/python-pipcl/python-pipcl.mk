################################################################################
#
# python-pipcl
#
################################################################################

PYTHON_PIPCL_VERSION = 12
PYTHON_PIPCL_SOURCE = pipcl-$(PYTHON_PIPCL_VERSION).tar.gz
PYTHON_PIPCL_SITE = https://files.pythonhosted.org/packages/64/1a/9ab2b272def9db9c80bf18fe8282119c2c4c074cc542030a28e4136dd13b
PYTHON_PIPCL_SETUP_TYPE = pep517
PYTHON_PIPCL_LICENSE = AGPL-3.0+
PYTHON_PIPCL_LICENSE_FILES = COPYING
HOST_PYTHON_PIPCL_DEPENDENCIES = host-python-packaging

$(eval $(host-python-package))
