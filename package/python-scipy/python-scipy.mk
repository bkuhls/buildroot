################################################################################
#
# python-scipy
#
################################################################################

PYTHON_SCIPY_VERSION = 1.18.1
PYTHON_SCIPY_SOURCE = scipy-$(PYTHON_SCIPY_VERSION).tar.gz
PYTHON_SCIPY_SITE = https://files.pythonhosted.org/packages/7e/74/66de6258867beb2ef08f35f9f2ac017a52cacd5081714d239ff1a442d458
PYTHON_SCIPY_SETUP_TYPE = meson
PYTHON_SCIPY_LICENSE = \
	BSD-3-Clause, \
	BSD-2-Clause, \
	BSD, \
	BSD-Style, \
	MIT, \
	Qhull
PYTHON_SCIPY_LICENSE_FILES = \
	LICENSE.txt \
	scipy/ndimage/LICENSE.txt \
	scipy/optimize/tnc/LICENSE \
	scipy/sparse/linalg/_dsolve/SuperLU/License.txt \
	scipy/sparse/linalg/_eigen/arpack/arnaud/LICENSE \
	scipy/sparse/linalg/_eigen/arpack/arnaud/ARPACK_LICENSE.txt \
	scipy/spatial/COPYING_QHULL.txt
PYTHON_SCIPY_CPE_ID_VENDOR = scipy
PYTHON_SCIPY_CPE_ID_PRODUCT = scipy
PYTHON_SCIPY_DEPENDENCIES += \
	host-python-cython \
	host-python-numpy \
	host-python-pythran \
	zlib \
	lapack \
	openblas \
	python-numpy \
	python-pybind
PYTHON_SCIPY_INSTALL_STAGING = YES
PYTHON_SCIPY_BUILD_OPTS = --skip-dependency-check

PYTHON_SCIPY_CONF_OPTS = -Dblas=openblas -Dlapack=lapack

PYTHON_SCIPY_MESON_EXTRA_PROPERTIES = \
	numpy-include-dir='$(STAGING_DIR)/usr/lib/python$(PYTHON3_VERSION_MAJOR)/site-packages/numpy/_core/include'

$(eval $(python-package))
