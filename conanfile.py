from conans import ConanFile, CMake, tools
from pathlib import Path
import subprocess
import os
from os import walk

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

    generators = "cmake_find_package_multi"
    build_policy = "missing"

    # Project Structure
    projectRoot = Path('.') / ".." / ".."
    projectRootString = str(projectRoot)

    projectBuild = projectRoot / "Build"
    projectBuildString = str(projectBuild)

    scm = {
        "type" : "git",
        "url" : "auto",
        "revision" : "auto"
    }

    no_copy_source = True

    requires = (
        "glfw/3.3",    
        "boost/1.71.0",
        "glm/0.9.9.5",
        "stb/20190512",
	    "flatbuffers/1.11.0",     
	    "imgui/1.69"    
    )

    build_requires = (
        "cmake_installer/3.17.2"
    )

    def build(self):

        cmake = CMake(self)
        cmake.configure()
        cmake.build()


    def package(self):

        pass


    def package_info(self):

        self.cpp_info.libs.append("SoulEngine")

        self.cpp_info.includedirs = [str(self.projectRoot / "Includes")]
        self.cpp_info.resdirs = [str(self.projectRoot / "Resources")]