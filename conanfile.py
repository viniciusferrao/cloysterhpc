from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMakeToolchain, CMake
from conan.tools.gnu import PkgConfigDeps

class MyProjectConan(ConanFile):
    name = "CloysterHPC"
    version = "0.1.1"
    settings = "os", "arch", "compiler", "build_type"

    def requirements(self):
        self.tool_requires("pkgconf/1.9.3")  # Ensure pkg-config is available
        self.requires("cli11/[>=2.4.0 <2.5.0]")
        self.requires("spdlog/[>=1.14.0 <1.15.0]")
        self.requires("fmt/[>=10.0.0 <12.0.0]")
        self.requires("boost/[>=1.83.0 <1.84.0]")
        self.requires("magic_enum/[>=0.9.0 <0.10.0]")
        self.requires("gsl-lite/[>=0.41.0 <0.42.0]")
        self.requires("doctest/[>=2.4.0 <2.5.0]")
        self.requires("sdbus-cpp/[>=2.0.0 <2.1.0]")
        # self.requires("glibmm/[>=2.78.1 <2.79.0]")

        # Override libmount to unify on 2.39.2.
        # This prevents the conflict with glib and sdbus-cpp requirements.
        self.requires("libmount/2.39.2", override=True)
        # Override libsystemd to fix compilation on RHEL 10.
        self.requires("libsystemd/[>=255.10 <256.0]", override=True)

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()
        pkg = PkgConfigDeps(self)
        pkg.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

