from conans import ConanFile

class CloysterHPC(ConanFile):
    # Note: options are copied from CMake boolean options.
    # When turned off, CMake sometimes passes them as empty strings.
#    options = {
#        "cpp_starter_use_imgui": ["ON", "OFF", ""],
#        "cpp_starter_use_sdl": ["ON", "OFF", ""]
#    }
    name = "CloysterHPC"
    version = "0.1"
    requires = (
        "boost/1.81.0",
        "fmt/9.1.0",
        "spdlog/1.11.0",
        "magic_enum/0.8.2",
        "gsl-lite/0.40.0"
#        "catch2/3.21",
#        "cli11/2.3.2",
    )
    generators = "cmake", "gcc", "txt", "cmake_find_package"

#    def requirements(self):
#        if self.options.cpp_starter_use_imgui == "ON":
#            self.requires("imgui-sfml/2.1@bincrafters/stable")
#        if self.options.cpp_starter_use_sdl == "ON":
#            self.requires("sdl2/2.0.10@bincrafters/stable")
