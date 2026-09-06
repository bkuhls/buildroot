################################################################################
#
# mesa-libclc
#
################################################################################

MESA_LIBCLC_VERSION = 22.1.8.3
MESA_LIBCLC_SITE = https://gitlab.freedesktop.org/karolherbst/mesa-libclc.git
MESA_LIBCLC_SITE_METHOD = git
MESA_LIBCLC_LICENSE = Apache-2.0 with exceptions or MIT
MESA_LIBCLC_LICENSE_FILES = LICENSE.TXT
MESA_LIBCLC_DEPENDENCIES = host-clang host-llvm host-spirv-llvm-translator
HOST_MESA_LIBCLC_DEPENDENCIES = host-clang host-llvm host-spirv-llvm-translator
MESA_LIBCLC_INSTALL_STAGING = YES

# CMAKE_*_COMPILER_FORCED=ON skips testing the tools and assumes
# llvm-config provided values
#
# CMAKE_*_COMPILER has to be set to the host compiler to build a host
# 'prepare_builtins' tool used during the build process
#
# The headers are installed in /usr/share and not /usr/include,
# because they are needed at runtime on the target to build the OpenCL
# kernels.
MESA_LIBCLC_CONF_OPTS = \
	-DCMAKE_SYSROOT="" \
	-DCMAKE_C_COMPILER_FORCED=ON \
	-DCMAKE_INSTALL_DATADIR="share" \
	-DCMAKE_FIND_ROOT_PATH="$(HOST_DIR)" \
	-DCMAKE_C_FLAGS="$(HOST_CFLAGS)" \
	-DCMAKE_EXE_LINKER_FLAGS="$(HOST_LDFLAGS)" \
	-DCMAKE_SHARED_LINKER_FLAGS="$(HOST_LDFLAGS)" \
	-DCMAKE_MODULE_LINKER_FLAGS="$(HOST_LDFLAGS)" \
	-DCMAKE_C_COMPILER="$(CMAKE_HOST_C_COMPILER)" \
	-DLLVM_CMAKE_DIR="$(HOST_DIR)/lib/cmake/llvm" \
	-DLIBCLC_CUSTOM_LLVM_TOOLS_BINARY_DIR="$(HOST_DIR)/bin"

HOST_MESA_LIBCLC_CONF_OPTS = \
	-DLIBCLC_TARGETS_TO_BUILD=spirv64-mesa3d-

$(eval $(cmake-package))
$(eval $(host-cmake-package))
