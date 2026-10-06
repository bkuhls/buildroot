import os

import infra.basetest

# The ARMv5 basic toolchain lacks always lock-free atomics, which most
# boost libraries depend on.
BOOST_CONFIG = \
    """
    BR2_arm=y
    BR2_cortex_a9=y
    BR2_ARM_ENABLE_VFP=y
    BR2_TOOLCHAIN_EXTERNAL=y
    BR2_TOOLCHAIN_EXTERNAL_BOOTLIN=y
    BR2_TOOLCHAIN_EXTERNAL_BOOTLIN_ARMV7_EABIHF_GLIBC_STABLE=y
    BR2_PACKAGE_BOOST=y
    BR2_PACKAGE_BOOST_RUNTIME_TEST=y
    BR2_TARGET_ROOTFS_CPIO=y
    # BR2_TARGET_ROOTFS_TAR is not set
    """


def boost_config(lib, extra=""):
    return BOOST_CONFIG + f"BR2_PACKAGE_BOOST_{lib.upper()}=y\n" + extra


# boost-mpi and boost-graph_parallel are not tested yet.
#
# Most tests are upstream tests built from the boost source tree and only
# checked for a zero exit code. Libraries without a suitable upstream test
# set expected_output and are built from the programs in boost-runtime-test/src.
class TestBoostBase(infra.basetest.BRTest):
    __test__ = False
    br2_external = [infra.filepath("tests/package/br2-external/boost")]
    config = BOOST_CONFIG
    lib = None
    expected_output = None

    def program(self):
        return "/usr/bin/boost-{}-test".format(self.lib.replace("_", "-"))

    def run_ok(self, cmd):
        output, exit_code = self.emulator.run(cmd)
        self.assertEqual(exit_code, 0, output)
        return output

    def check(self):
        output = self.run_ok(self.program())
        if self.expected_output is not None:
            self.assertEqual(output, self.expected_output)

    def test_run(self):
        cpio_file = os.path.join(self.builddir, "images", "rootfs.cpio")
        self.emulator.boot(arch="armv7",
                           kernel="builtin",
                           options=["-initrd", cpio_file])
        self.emulator.login()
        self.check()


class TestBoostAtomic(TestBoostBase):
    __test__ = True
    lib = "atomic"
    config = boost_config(lib)


class TestBoostChrono(TestBoostBase):
    __test__ = True
    lib = "chrono"
    config = boost_config(lib)


class TestBoostContainer(TestBoostBase):
    __test__ = True
    lib = "container"
    config = boost_config(lib)


class TestBoostContext(TestBoostBase):
    __test__ = True
    lib = "context"
    config = boost_config(lib)


class TestBoostContract(TestBoostBase):
    __test__ = True
    lib = "contract"
    config = boost_config(lib)


class TestBoostCoroutine(TestBoostBase):
    __test__ = True
    lib = "coroutine"
    config = boost_config(lib)
    expected_output = ["coroutine: fib(9)=34"]


class TestBoostDateTime(TestBoostBase):
    __test__ = True
    lib = "date_time"
    config = boost_config(lib)


class TestBoostException(TestBoostBase):
    __test__ = True
    lib = "exception"
    config = boost_config(lib)


class TestBoostFiber(TestBoostBase):
    __test__ = True
    lib = "fiber"
    config = boost_config(lib)
    expected_output = ["fiber: sum=15"]


class TestBoostFilesystem(TestBoostBase):
    __test__ = True
    lib = "filesystem"
    config = boost_config(lib)


class TestBoostGraph(TestBoostBase):
    __test__ = True
    lib = "graph"
    config = boost_config(lib)


class TestBoostIostreams(TestBoostBase):
    __test__ = True
    lib = "iostreams"
    config = boost_config(lib)
    expected_output = ["iostreams: gzip and bzip2 round trip OK"]


class TestBoostJson(TestBoostBase):
    __test__ = True
    lib = "json"
    config = boost_config(lib)


class TestBoostLocale(TestBoostBase):
    __test__ = True
    lib = "locale"
    config = boost_config(lib, 'BR2_GENERATE_LOCALE="en_US.UTF-8"\n'
                               "BR2_TOOLCHAIN_GLIBC_GCONV_LIBS_COPY=y\n")
    expected_output = ["locale: 5 characters"]


class TestBoostLog(TestBoostBase):
    __test__ = True
    lib = "log"
    config = boost_config(lib, "BR2_PACKAGE_BOOST_TEST=y\n")


class TestBoostMath(TestBoostBase):
    __test__ = True
    lib = "math"
    config = boost_config(lib, "BR2_PACKAGE_BOOST_TEST=y\n")


class TestBoostNowide(TestBoostBase):
    __test__ = True
    lib = "nowide"
    config = boost_config(lib)


class TestBoostProgramOptions(TestBoostBase):
    __test__ = True
    lib = "program_options"
    config = boost_config(lib)


class TestBoostPython(TestBoostBase):
    __test__ = True
    lib = "python"
    config = boost_config(lib, "BR2_PACKAGE_PYTHON3=y\n")

    def check(self):
        cmd = "python3 -c 'import boost_python_test as m; c = m.Counter(); " \
              "c.add(3); c.add(4); print(m.greet(\"buildroot\"), c.value)'"
        self.assertEqual(self.run_ok(cmd), ["hello buildroot 7"])


class TestBoostRandom(TestBoostBase):
    __test__ = True
    lib = "random"
    config = boost_config(lib, "BR2_PACKAGE_BOOST_TEST=y\n")


class TestBoostRegex(TestBoostBase):
    __test__ = True
    lib = "regex"
    config = boost_config(lib)


class TestBoostSerialization(TestBoostBase):
    __test__ = True
    lib = "serialization"
    config = boost_config(lib)


class TestBoostStacktrace(TestBoostBase):
    __test__ = True
    lib = "stacktrace"
    config = boost_config(lib)


class TestBoostSystem(TestBoostBase):
    __test__ = True
    lib = "system"
    config = boost_config(lib)


class TestBoostTest(TestBoostBase):
    __test__ = True
    lib = "test"
    config = boost_config(lib)


class TestBoostThread(TestBoostBase):
    __test__ = True
    lib = "thread"
    config = boost_config(lib)


class TestBoostTimer(TestBoostBase):
    __test__ = True
    lib = "timer"
    config = boost_config(lib)


class TestBoostTypeErasure(TestBoostBase):
    __test__ = True
    lib = "type_erasure"
    config = boost_config(lib, "BR2_PACKAGE_BOOST_TEST=y\n")


class TestBoostUrl(TestBoostBase):
    __test__ = True
    lib = "url"
    config = boost_config(lib)
    expected_output = ["url: https://buildroot.org:8080/downloads/index.html?ver=2026.11&arch=aarch64"]


class TestBoostWave(TestBoostBase):
    __test__ = True
    lib = "wave"
    config = boost_config(lib)
    expected_output = ["wave: hello buildroot"]
