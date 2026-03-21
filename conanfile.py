from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import copy
import os


class HsmcppConan(ConanFile):
    name = "hsmcpp"
    version = "1.0.2"
    package_type = "library"

    description = "C++ library for hierarchical/finite state machines"
    license = "MIT"
    url = "https://github.com/igor-krechetov/hsmcpp"
    homepage = "https://hsmcpp.readthedocs.io"
    topics = ("fsm", "hsm", "state-machine", "embedded", "scxml")

    settings = "os", "compiler", "build_type", "arch"

    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_std_dispatcher": [True, False],
        "enable_thread_safety": [True, False],
        "enable_debugging": [True, False],
        "enable_structure_validation": [True, False],
        "enable_verbose_logging": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_std_dispatcher": True,
        "enable_thread_safety": True,
        "enable_debugging": True,
        "enable_structure_validation": True,
        "enable_verbose_logging": False,
    }

    exports_sources = (
        "CMakeLists.txt",
        "cmake/*",
        "include/*",
        "src/*",
        "tools/*",
        "pkgconfig/*",
    )

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def layout(self):
        cmake_layout(self)

    def validate(self):
        if self.options.shared:
            raise ConanInvalidConfiguration("hsmcpp Conan recipe currently supports static library packaging only (shared=False).")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["HSMBUILD_TARGET"] = "library"
        tc.variables["HSMBUILD_PLATFORM"] = self._get_hsmbuild_platform()
        tc.variables["HSMBUILD_TESTS"] = "OFF"
        tc.variables["HSMBUILD_EXAMPLES"] = "OFF"
        tc.variables["HSMBUILD_DISPATCHER_STD"] = "ON" if self.options.with_std_dispatcher else "OFF"
        tc.variables["HSMBUILD_THREAD_SAFETY"] = "ON" if self.options.enable_thread_safety else "OFF"
        tc.variables["HSMBUILD_DEBUGGING"] = "ON" if self.options.enable_debugging else "OFF"
        tc.variables["HSMBUILD_STRUCTURE_VALIDATION"] = "ON" if self.options.enable_structure_validation else "OFF"
        tc.variables["HSMBUILD_VERBOSE"] = "ON" if self.options.enable_verbose_logging else "OFF"
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def _get_hsmbuild_platform(self):
        if str(self.settings.os) == "Windows":
            return "windows"

        # Linux/FreeBSD/QNX(Neutrino) and other POSIX-like targets.
        return "posix"

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

        copy(self, "LICENSE", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))

    def package_info(self):
        self.cpp_info.libs = ["hsmcpp"]

        if self.options.with_std_dispatcher:
            self.cpp_info.libs.append("hsmcpp_std")

        if str(self.settings.os) in ["Linux", "FreeBSD", "Neutrino"]:
            self.cpp_info.system_libs.append("pthread")

        self.cpp_info.set_property("cmake_file_name", "hsmcpp")
        self.cpp_info.set_property("cmake_target_name", "hsmcpp::hsmcpp")
