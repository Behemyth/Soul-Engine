cmake_minimum_required(VERSION 3.15.2 FATAL_ERROR)

#Set CMake tools location
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/tools/CMake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}) #TODO: remove with new Conan generator
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}) #TODO: remove with new Conan generator

#Global Options
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON) # Opposite module/PCF order for `find_package`
set_property(GLOBAL PROPERTY USE_FOLDERS ON) # Group the default targets in a folder


##############################################
#Project 

project(SoulEngine 
	VERSION 0.0.1
	DESCRIPTION "Real-time simulation and rendering engine."
	LANGUAGES CXX
)


##############################################
#Options

option(BUILD_TESTS "Build test projects" OFF)
option(BUILD_EXAMPLES "Build example projects" ON)
option(BUILD_SHARED_LIBS "Build shared library" OFF)


##############################################
#Project Target

add_library(SoulEngine)
add_library(synodic::SoulEngine ALIAS SoulEngine)

set_target_properties(SoulEngine
PROPERTIES 
	LINKER_LANGUAGE CXX
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS NO
)


##############################################
#Sources

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*)

target_include_directories(SoulEngine
PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_sources(SoulEngine
PRIVATE
	${SOURCES}
)

#Provides Visual Studio filter support
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES SOURCES)


##############################################
#Subprojects

if(BUILD_TESTS)

    add_subdirectory(Tests)

endif()

if(BUILD_EXAMPLES)

    add_subdirectory(Examples)

endif()
