import subprocess
import sys
import os

from pathlib import Path
from Utilities import Task


def Update():

    print("Updating the Python environment...")

    Task("poetry", "update")

    print("Updating the C++ environment...")

    buildPath = Path().absolute() / "Build"
    buildPathString = str(buildPath)

    conanFilePath = Path().absolute() / "Tools" / "Conan"
    conanFilePathString = str(conanFilePath)

    #update conan dependencies
    Task("conan", "graph", "lock", conanFilePathString, "-l", conanFilePathString)
    Task("conan", "install", conanFilePathString, "-if", buildPathString,"-l", conanFilePathString, "-s", "build_type=Debug")
    Task("conan", "install", conanFilePathString, "-if", buildPathString,"-l", conanFilePathString, "-s", "build_type=Release")


    print("Finished update")