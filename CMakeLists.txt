cmake_minimum_required(VERSION 3.13.1 FATAL_ERROR)

#Set CMake tools location
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/Tools/CMake)


##############################################
#Project 

project(SoulEngine 
	VERSION 0.0.1
	DESCRIPTION "Real-time simulation and rendering engine."
	LANGUAGES CXX
)


##############################################
#Dependencies

include(CheckLanguage)

#Conan via CMake
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Build/conanbuildinfo_multi.cmake)
    #The building case
    include(${CMAKE_CURRENT_SOURCE_DIR}/Build/conanbuildinfo_multi.cmake)
else()
    #The packaging case
    include(${CMAKE_CURRENT_SOURCE_DIR}/Build/conanbuildinfo.cmake)
endif()

#if this repository is used via `add_subdirectory` we can't double initialize Conan packages
if(NOT TARGET CONAN_PKG::SoulEngine)

    conan_basic_setup(TARGETS)

endif()


##############################################
#Options

option(BUILD_TESTS "Build test projects" OFF)#TODO: Implement testing framework. Once that is done, this option can be defaulted to ON
option(BUILD_EXAMPLES "Build example projects" ON)
option(BUILD_SHARED_LIBS "Build shared library" OFF)#TODO: Implement use of this option


##############################################
#Project Target

add_library(${PROJECT_NAME} "")
add_library(synodic::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME}
	PROPERTIES 
        LINKER_LANGUAGE CXX
	   	CXX_EXTENSIONS OFF	
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
		USE_FOLDERS ON
)

target_include_directories(${PROJECT_NAME}
    PUBLIC 
        $<INSTALL_INTERFACE:Includes>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Includes>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        CONAN_PKG::glm

    PRIVATE	
        CONAN_PKG::boost
        CONAN_PKG::stb
        CONAN_PKG::imgui
)


##############################################
#Sources

target_sources(${PROJECT_NAME}
    PUBLIC
        Includes/Property.h
        Includes/Soul.h
        Includes/SoulApplication.h
        Includes/SoulParameters.h
        Includes/Types.h
        Includes/WindowParameters.h    
)

#Each `Source` subdirectory handles its own sources and compilation choices
add_subdirectory(Source/Audio)
add_subdirectory(Source/Compute)
add_subdirectory(Source/Core)
add_subdirectory(Source/Display/Window)
add_subdirectory(Source/Display/Input)
add_subdirectory(Source/Display/GUI)
add_subdirectory(Source/Memory)
add_subdirectory(Source/Network)
add_subdirectory(Source/Parallelism/Graph)
add_subdirectory(Source/Parallelism/Scheduler)
add_subdirectory(Source/Physics)
add_subdirectory(Source/Render/Raster)
add_subdirectory(Source/Render/RenderGraph)
add_subdirectory(Source/Tracer)
add_subdirectory(Source/Transput/Resource)

#Provides Visual Studio filter support
get_target_property(PROJECT_SOURCES ${PROJECT_NAME} SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_SOURCES})


##############################################
#Subprojects

if(BUILD_TESTS)

    add_subdirectory(Tests)

endif()

if(BUILD_EXAMPLES)

    add_subdirectory(Examples)

endif()
