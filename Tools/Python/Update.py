import subprocess
import sys
import os

from pathlib import Path
from Utilities import Task


def Update():

    Task("Poetry: Update From Lockfile", "poetry", "install")

    buildPath = Path().absolute() / "Build"
    buildPathString = str(buildPath)

    conanFilePath = Path().absolute() / "Tools" / "Conan"
    conanFilePathString = str(conanFilePath)

    #update conan dependencies
    Task("Conan: Install Debug Dependencies", "conan", "install", conanFilePathString, "-if", buildPathString,"-l", conanFilePathString, "-s", "build_type=Debug")
    Task("Conan: Install Release Dependencies", "conan", "install", conanFilePathString, "-if", buildPathString,"-l", conanFilePathString, "-s", "build_type=Release")