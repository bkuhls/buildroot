################################################################################
#
# python-iso-639
#
################################################################################

PYTHON_ISO_639_VERSION = 0.4.5
PYTHON_ISO_639_SOURCE = iso-639-$(PYTHON_ISO_639_VERSION).tar.gz
PYTHON_ISO_639_SITE = https://pypi.python.org/packages/5a/8d/27969852f4e664525c3d070e44b2b719bc195f4d18c311c52e57bb93614e
PYTHON_ISO_639_SETUP_TYPE = setuptools
PYTHON_ISO_639_LICENSE = GNU Affero General Public License v3

$(eval $(python-package))
