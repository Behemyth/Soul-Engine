import subprocess
import sys
import os

from pathlib import Path
from Utilities import Task


def Setup():
    
    print("Installing the C++ environment...")

    buildPath = Path().absolute() / "Build"
    buildPathString = str(buildPath)

    sourcePath = Path().absolute()
    sourcePathString = str(sourcePath)

    conanFilePath = Path().absolute() / "Tools" / "Conan"
    conanFilePathString = str(conanFilePath)

    #Set the conan remote
    Task("conan", "remote", "add", "--force", "bincrafters", "https://api.bintray.com/conan/bincrafters/public-conan")

    #Create build directory if it does not exist
    if not os.path.exists(buildPath):
        os.makedirs(buildPath)

    #install conan dependencies
    Task("conan", "install", conanFilePathString, "-if", buildPathString,"-l", conanFilePathString, "-s", "build_type=Debug")
    Task("conan", "install", conanFilePathString, "-if", buildPathString,"-l", conanFilePathString, "-s", "build_type=Release")

    #TODO: Grab the required engine version dependency from `conanfile.txt`
    #set the package to editable, allowing projects to find it globally via Conan and bypass a remote fetch
    Task("conan", "editable", "add", conanFilePathString, "SoulEngine/0.0.1@synodic/testing")

    # print("Building Soul Engine...")

    # Task()"conan", "build", conanFilePathString, "-bf", buildPathString, "-sf", sourcePathString, "--configure")


    print("Finished setup")