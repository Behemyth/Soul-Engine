# Setting Up Your Development Environment

Welcome!

If you are developing an application that uses Soul Engine and you are not going to build the engine yourself, skip ahead to [Developing an Application *Without* Soul Engine](#developing-an-application-without-soul-engine)

## Compilers

CUDA has specific compilers that it is compatible with. Make sure the compiler you choose is listed in CUDA's documentation.

### *Windows*

- It is ***highly*** suggested that you use [Visual Studio 2019](https://visualstudio.microsoft.com/downloads/).

### *OSX*

- **TODO** - Contributions Welcome

### *Linux*

- **TODO** - Contributions Welcome

## Dependencies
In order to use Soul Engine you will have to download and install some dependencies with the listed minimum versions.  See [General Installation Workflow ](#general-installation-Workflow) for your specific system instructions.

### Manual Dependencies
Look for the following dependencies in your OS specific installation instructions:

* [CMake 3.14.1](https://cmake.org/download/)
* [Python 3.7.2](https://www.python.org/downloads/release/python-372/)
* [Conan 1.16.0](https://conan.io/downloads.html)
* [Vulkan SDK 1.1.108.0](https://vulkan.lunarg.com/sdk/home)
* [CUDA 10.1](https://developer.nvidia.com/cuda-downloads)

### Automatic Dependencies
The following dependencies are satisfied automatically:

* [GLFW](https://github.com/glfw/glfw)
* [GLM](https://github.com/g-truc/glm)
* [stb_image.h](https://github.com/nothings/stb)
* [Boost](https://www.boost.org/)
* [FlatBuffers](https://google.github.io/flatbuffers/)
* [dear imgui](https://github.com/ocornut/imgui)


# General Installation Workflow 

You have three choices when using and developing Soul Engine.

1. You can [just develop Soul Engine](#developing-just-soul-engine) and use the provided example application to execute it. 

1. You can [develop an application alongside the engine](#developing-an-application-with-soul-engine).

1. You can [just develop an application](#developing-an-application-without-soul-engine) without downloading the engine's source files.

## Developing *Just* Soul Engine 


### *Windows*

1.  Install the project's dependencies.
    * [CMake 3.14.1](https://cmake.org/download/) Download and install. This is only needed if not using the integrated CMake with Visual Studio 2019.
    * [Python 3.7.2](https://www.python.org/downloads/release/python-372/) Download and install. Add Python to the system path via the installer option.
    * [Conan 1.16.0](https://conan.io/downloads.html) Install via Python. `pip install conan`
    * [Vulkan SDK 1.1.108.0](https://vulkan.lunarg.com/sdk/home) Download and install.
    * [CUDA 10.1](https://developer.nvidia.com/cuda-downloads) Download and install.

1. Run `Setup.py`

1. You have two options to generate the project via CMake.
    * Open the `C:\Projects\Soul Engine` directory in Visual Studio 2019 to use the integrated CMake functionality. 
    * Generate the project with CMake. The location of the build can be found at `C:\Projects\Soul Engine\Build`.

1. Build and run `Soul Info`.

### *OSX*

1. **TODO** - Contributions Welcome


### *Linux*

1. **TODO** - Contributions Welcome

## Developing an Application *With* Soul Engine

### *Windows*


1.  Install the project's dependencies.
    * [CMake 3.14.1](https://cmake.org/download/) Download and install. This is only needed if not using the integrated CMake with Visual Studio 2019.
    * [Python 3.7.2](https://www.python.org/downloads/release/python-372/) Download and install. Add Python to the system path via the installer option.
    * [Conan 1.16.0](https://conan.io/downloads.html) Install via Python. `pip install conan`
    * [Vulkan SDK 1.1.108.0](https://vulkan.lunarg.com/sdk/home) Download and install.
    * [CUDA 10.1](https://developer.nvidia.com/cuda-downloads) Download and install.

1. Run `Setup.py`

1. Clone or download an application. For example [Soul Editor](https://github.com/Synodic-Software/Soul-Editor).

1. You have two options to generate the project via CMake.
    * Open the `C:\Projects\Soul Editor` directory in Visual Studio 2019 to use the integrated CMake functionality. 
    * Generate the application with CMake. The location of the build can be found at `C:\Projects\Soul Editor\Build`.

1. Build and run `Soul Editor`.

### *OSX*

1. **TODO** - Contributions Welcome


### *Linux*

1. **TODO** - Contributions Welcome


## Developing an Application *Without* Soul Engine

### *Windows*

1. **TODO: This Feature Is NOT Completed** - Contributions Welcome

1.  Install the application's manual dependencies. The minimum requirements are listed here:
    * [CMake 3.14.1](https://cmake.org/download/) Download and install. This is only needed if not using the integrated CMake with Visual Studio 2019.
    * [Python 3.7.2](https://www.python.org/downloads/release/python-372/) Download and install. Add Python to the system path via the installer option.
    * [Conan 1.16.0](https://conan.io/downloads.html) Install via Python. `pip install conan`

1. Clone or download an application. For example [Soul Editor](https://github.com/Synodic-Software/Soul-Editor).

1. You have two options to generate the project via CMake.
    * Open the `C:\Projects\Soul Editor` directory in Visual Studio 2019 to use the integrated CMake functionality. 
    * Generate the application with CMake. The location of the build can be found at `C:\Projects\Soul Editor\Build`.

1. Build and run `Soul Editor`.

### *OSX*

1. **TODO** - Contributions Welcome


### *Linux*

1. **TODO** - Contributions Welcome