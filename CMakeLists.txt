cmake_minimum_required(VERSION 3.17.2 FATAL_ERROR)

#Set CMake tools location
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/tools/CMake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}) #TODO: remove with new Conan generator
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}) #TODO: remove with new Conan generator

#Global Options
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON) # Opposite module/PCF order for `find_package`
set_property(GLOBAL PROPERTY USE_FOLDERS ON) # Group the default targets in a folder


##############################################
#Project 

project(soul-engine 
	VERSION 0.1.0
	DESCRIPTION "Real-time simulation and rendering engine."
	LANGUAGES CXX
)


##############################################
#Options

option(BUILD_TESTS "Build test projects" ON)
option(BUILD_EXAMPLES "Build example projects" ON)
option(BUILD_SHARED_LIBS "Build shared library" OFF)


##############################################
#Project Target

add_library(${PROJECT_NAME})
add_library(synodic::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME}
PROPERTIES 
	LINKER_LANGUAGE CXX
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS NO
)


##############################################
# Dependencies

find_package(cppaste 0.1.0 REQUIRED)
find_package(tracer 0.1.0 REQUIRED)
find_package(glm 0.9.9.8 REQUIRED)
find_package(acceleration-structure 0.1.0 REQUIRED)


##############################################
#Sources

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*)

target_include_directories(${PROJECT_NAME}
PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_sources(${PROJECT_NAME}
PRIVATE
	${SOURCES}
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
	tracer::tracer
	glm::glm
PRIVATE
	cppaste::cppaste
	acceleration-structure::acceleration-structure
)

#Provides Visual Studio filter support
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES SOURCES)


##############################################
#Subprojects

if(BUILD_TESTS)

    add_subdirectory(tests)

endif()

if(BUILD_EXAMPLES)

    add_subdirectory(examples)

endif()
