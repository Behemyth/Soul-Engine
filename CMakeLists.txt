cmake_minimum_required(VERSION 3.15.2 FATAL_ERROR)

#Set CMake tools location
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Tools/CMake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}) #TODO: remove with new Conan generator
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}) #TODO: remove with new Conan generator

#Global Options
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON) # Opposite module/PCF order for `find_package`

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
find_package(imgui REQUIRED)

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
        glm::glm

    PRIVATE	
        boost::boost
        stb::stb
        imgui::imgui
)


##############################################
#Sources

#TODO: Includes should exist in their own modules
#Public Includes
file(GLOB_RECURSE PUBLIC_SOURCES CONFIGURE_DEPENDS Includes/*)

target_sources(${PROJECT_NAME}
    PUBLIC
        ${PUBLIC_SOURCES}
)


#Search for all modules
FindModules(MODULES ${CMAKE_CURRENT_SOURCE_DIR}/Source)

foreach(MODULE ${MODULES})

    add_subdirectory(${MODULE})

endforeach()


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
