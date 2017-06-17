################################################################################
#
# python-backports-shutil-which
#
################################################################################

PYTHON_BACKPORTS_SHUTIL_WHICH_VERSION = 3.5.1
PYTHON_BACKPORTS_SHUTIL_WHICH_SOURCE = backports.shutil_which-$(PYTHON_BACKPORTS_SHUTIL_WHICH_VERSION).tar.gz
PYTHON_BACKPORTS_SHUTIL_WHICH_SITE = https://pypi.python.org/packages/dd/ea/715dc80584207a0ff4a693a73b03c65f087d8ad30842832b9866fe18cb2f
PYTHON_BACKPORTS_SHUTIL_WHICH_SETUP_TYPE = distutils
PYTHON_BACKPORTS_SHUTIL_WHICH_LICENSE = MIT
PYTHON_BACKPORTS_SHUTIL_WHICH_LICENSE_FILE = LICENSE

$(eval $(python-package))

