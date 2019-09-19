import subprocess
import sys
import os

from pathlib import Path
from Utilities import Task


def Setup():

    buildPath = Path().absolute() / "Build"
    buildPathString = str(buildPath)

    sourcePath = Path().absolute()
    sourcePathString = str(sourcePath)

    conanFilePath = Path().absolute() / "Tools" / "Conan"
    conanFilePathString = str(conanFilePath)

    conanLockPathDebug = Path().absolute() / "Tools" / "Conan" / "Debug"
    conanLockPathDebugString = str(conanLockPathDebug)

    conanLockPathRelease = Path().absolute() / "Tools" / "Conan" / "Release"
    conanLockPathReleaseString = str(conanLockPathRelease)

    #Set the conan remote
    Task("Conan: Set Remotes", "conan", "remote", "add", "--force", "bincrafters", "https://api.bintray.com/conan/bincrafters/public-conan")

    #Create build directory if it does not exist
    if not os.path.exists(buildPath):
        os.makedirs(buildPath)

    #install conan dependencies
    Task("Conan: Install Debug Dependencies", "conan", "install", conanFilePathString, "-if", buildPathString,"-l", conanLockPathDebugString, "-s", "build_type=Debug")
    Task("Conan: Install Release Dependencies", "conan", "install", conanFilePathString, "-if", buildPathString,"-l", conanLockPathReleaseString, "-s", "build_type=Release")

    #TODO: Grab the required engine version dependency from `conanfile.txt`
    #set the package to editable, allowing projects to find it globally via Conan and bypass a remote fetch
    Task("Conan: Enable Editable Package", "conan", "editable", "add", conanFilePathString, "SoulEngine/0.0.1@synodic/testing")

    # print("Building Soul Engine...")

    # Task("conan", "build", conanFilePathString, "-bf", buildPathString, "-sf", sourcePathString, "--configure")