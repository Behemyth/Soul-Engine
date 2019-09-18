cmake_minimum_required(VERSION 3.15.2 FATAL_ERROR)

#Set CMake tools location
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Tools/CMake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}) #TODO: remove with new Conan generator
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}) #TODO: remove with new Conan generator

#Global Options
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON) # Opposite module/PCF order for `find_package`
set_property(GLOBAL PROPERTY USE_FOLDERS ON) # Group the default targets in a folder

include(Module)


##############################################
#Project 

project(SoulEngine 
	VERSION 0.0.1
	DESCRIPTION "Real-time simulation and rendering engine."
	LANGUAGES CXX
)


##############################################
#Dependencies

find_package(glm REQUIRED)
find_package(boost REQUIRED)
find_package(stb REQUIRED)


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
	   	CXX_EXTENSIONS OFF	
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
)

target_include_directories(SoulEngine
    PUBLIC 
        $<INSTALL_INTERFACE:Include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>
)

target_link_libraries(SoulEngine
    PUBLIC
        glm::glm

    PRIVATE	
        boost::boost
        stb::stb
)


##############################################
#Sources

#Search for all modules
FindModules(MODULES ${CMAKE_CURRENT_SOURCE_DIR}/Source PROJECT_SOURCES)

target_sources(SoulEngine
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Include/Soul.h
)

#Provides Visual Studio filter support
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_SOURCES})

#Link modules to the engine
target_link_libraries(SoulEngine
PRIVATE
    ${MODULES}
)


##############################################
#Subprojects

if(BUILD_TESTS)

    add_subdirectory(Tests)

endif()

if(BUILD_EXAMPLES)

    add_subdirectory(Examples)

endif()
