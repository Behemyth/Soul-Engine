from conans import ConanFile, CMake, tools
from pathlib import Path

import os

class SoulEngine(ConanFile):

    name = "SoulEngine"
    version = "0.0.1"
    author = "Synodic Software"
    license = "GPLv3"
    url = "https://github.com/Synodic-Software/Soul-Engine"
    description = "Soul Engine is a real-time visualization and simulation engine built on the back of CUDA, OpenCL, and Vulkan."

    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {"shared": False}

    generators = "cmake"

    scm = {
        "type" : "git",
        "url" : "auto",
        "subfolder": "../..",
        "revision" : "auto"
    }

    no_copy_source = True

    requires = (
        "glfw/3.3@bincrafters/stable",    
        "boost/1.71.0@conan/stable",
        "glm/0.9.9.5@g-truc/stable",
        "stb/20190512@conan/stable",
	    "flatbuffers/1.11.0@google/stable",     
	    "imgui/1.69@bincrafters/stable"    
    )

    def configureCMake(self):
        
        cmake = CMake(self)
        cmake.configure(source_folder="../..")

        return cmake


    def build(self):

        cmake = self.configureCMake()
        cmake.build()


    def package(self):

        pass


    def package_info(self):

        self.cpp_info.libs = ["SoulEngine"]

        projectRoot = Path('.') / ".." / ".."
        self.cpp_info.includedirs = [str(projectRoot / "Includes")]
        self.cpp_info.resdirs = [str(projectRoot / "Resources")]

        #CMAKE environment variables
        self.user_info.ENGINE_PATH = str(projectRoot.absolute())