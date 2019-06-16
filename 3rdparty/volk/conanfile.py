import os

from conans import ConanFile, CMake

class VolkConan(ConanFile):
    name = "volk"
    version = "1.0.0"
    license = "MIT"
    author = "Arseny Kapoulkine"
    url = "https://github.com/zeux/volk"
    description = "volk is a meta-loader for Vulkan."
    settings = "os", "compiler", "build_type", "arch"
    options = {}
    default_options = {}
    exports_sources = "src/*"

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder="./src")
        cmake.build()

    def package(self):
        self.copy("*.h", dst="include", src="src/", keep_path=True)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["volk"]
