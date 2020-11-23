from conans import ConanFile, CMake, tools
import os, shutil

class CliCppConan(ConanFile):
    name = "cli"
    version = "1.2.1"
    license = "Boost Software License 1.0"
    url = "https://github.com/daniele77/cli"
    description = "A cross-platform header only C++14 library for interactive command line interfaces (Cisco style)"
    settings = ("os", "build_type", "arch", "compiler")
    generators = "cmake"
    exports_sources = "doc/**", "include/**", "examples/**", "test/**", "CONTRIBUTING.md","README.md","CMakeLists.txt","LICENSE","cli.pc.in","cliConfig.cmake.in","CHANGELOG.md","CODE_OF_CONDUCT.md"

    options = { "use_boost": [True, False] }
    default_options = {"use_boost": False }

    def build(self):
        cmake = CMake(self)
        
        if self.options.use_boost:
            cmake.definitions["CLI_UseBoost"] = "ON"
        cmake.definitions["CLI_UseConan"] = "ON"
        
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy("*.h", dst="include", src="include", keep_path=True)
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.name = "cli"
#        self.cpp_info.names["generator_name"] = "<PKG_NAME>"
        self.cpp_info.includedirs = ['include']  # Ordered list of include paths
        self.cpp_info.libs = tools.collect_libs(self)  # The libs to link against
        self.cpp_info.system_libs = []  # System libs to link against
        self.cpp_info.libdirs = ['.', 'lib']  # Directories where libraries can be found
        self.cpp_info.resdirs = ['res']  # Directories where resources, data, etc. can be found
        self.cpp_info.bindirs = ['bin']  # Directories where executables and shared libs can be found
        self.cpp_info.srcdirs = []  # Directories where sources can be found (debugging, reusing sources)
        self.cpp_info.build_modules = []  # Build system utility module files
        self.cpp_info.defines = []  # preprocessor definitions
        self.cpp_info.cflags = []  # pure C flags
        self.cpp_info.cxxflags = []  # C++ compilation flags
        self.cpp_info.sharedlinkflags = []  # linker flags
        self.cpp_info.exelinkflags = []  # linker flags
        self.cpp_info.components  # Dictionary with the different components a package may have