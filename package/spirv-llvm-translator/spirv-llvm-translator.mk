################################################################################
#
# spirv-llvm-translator
#
################################################################################

# Generate version string using:
#   git describe --tags --match 'v21*' --abbrev=40 origin/llvm_release_210
SPIRV_LLVM_TRANSLATOR_VERSION = v21.1.1-1-g085f2d9b94759b01c79da57616b35a77a7b8e90f
SPIRV_LLVM_TRANSLATOR_SITE = $(call github,KhronosGroup,SPIRV-LLVM-Translator,$(SPIRV_LLVM_TRANSLATOR_VERSION))
SPIRV_LLVM_TRANSLATOR_LICENSE = NCSA
SPIRV_LLVM_TRANSLATOR_LICENSE_FILES = LICENSE.TXT
HOST_SPIRV_LLVM_TRANSLATOR_DEPENDENCIES = host-clang host-llvm host-spirv-headers
HOST_SPIRV_LLVM_TRANSLATOR_CONF_OPTS = \
	-DLLVM_BUILD_TOOLS=ON \
	-DLLVM_DIR=$(HOST_DIR)/lib/cmake/llvm \
	-DLLVM_SPIRV_BUILD_EXTERNAL=YES \
	-DLLVM_SPIRV_INCLUDE_TESTS=OFF \
	-DLLVM_EXTERNAL_PROJECTS="SPIRV-Headers" \
	-DLLVM_EXTERNAL_SPIRV_HEADERS_SOURCE_DIR=$(HOST_DIR)/include

SPIRV_LLVM_TRANSLATOR_INSTALL_STAGING = YES
SPIRV_LLVM_TRANSLATOR_DEPENDENCIES = host-clang host-llvm llvm spirv-headers
SPIRV_LLVM_TRANSLATOR_CONF_OPTS = \
	-DLLVM_EXTERNAL_SPIRV_HEADERS_SOURCE_DIR=$(STAGING_DIR)/usr \
	-DLLVM_BUILD_TOOLS=ON \
	-DLLVM_DIR=$(STAGING_DIR)/lib/cmake/llvm \
	-DLLVM_SPIRV_BUILD_EXTERNAL=YES \
	-DLLVM_SPIRV_INCLUDE_TESTS=OFF

$(eval $(cmake-package))
$(eval $(host-cmake-package))
