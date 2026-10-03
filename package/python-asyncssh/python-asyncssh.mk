################################################################################
#
# python-asyncssh
#
################################################################################

PYTHON_ASYNCSSH_VERSION = 2.24.0
PYTHON_ASYNCSSH_SOURCE = asyncssh-$(PYTHON_ASYNCSSH_VERSION).tar.gz
PYTHON_ASYNCSSH_SITE = https://files.pythonhosted.org/packages/dd/7f/2d79247bacc562104f312d27efe541673aa177feac89de291bd61bca52be
PYTHON_ASYNCSSH_SETUP_TYPE = setuptools
PYTHON_ASYNCSSH_LICENSE = EPL-2.0 or GPL-2.0+
PYTHON_ASYNCSSH_LICENSE_FILES = LICENSE COPYRIGHT
PYTHON_ASYNCSSH_CPE_ID_VENDOR = asyncssh_project
PYTHON_ASYNCSSH_CPE_ID_PRODUCT = asyncssh

$(eval $(python-package))
