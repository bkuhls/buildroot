################################################################################
#
# python-streamlink
#
################################################################################

PYTHON_STREAMLINK_VERSION = 0.6.0
PYTHON_STREAMLINK_SOURCE = streamlink-$(PYTHON_STREAMLINK_VERSION).tar.gz
PYTHON_STREAMLINK_SITE = https://pypi.python.org/packages/29/dc/959bcae094e2afe1f26d4b8a129d3989ec0470069b87797cb64d6be11f7b
PYTHON_STREAMLINK_SETUP_TYPE = setuptools
PYTHON_STREAMLINK_LICENSE = Simplified BSD
PYTHON_STREAMLINK_LICENSE_FILES = LICENSE docs/_themes/sphinx_rtd_theme_violet/LICENSE

$(eval $(python-package))
