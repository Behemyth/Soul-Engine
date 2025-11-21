cmake_minimum_required(VERSION 4.0 FATAL_ERROR)

# Enable std module support for MSVC - MUST be before project()
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")

set(CMAKE_CXX_MODULE_STD ON)

project(Sample)

add_executable(sample)

set_target_properties(sample
	PROPERTIES
        LINKER_LANGUAGE CXX
	   	CXX_EXTENSIONS OFF
        CXX_STANDARD 23
)

target_link_libraries(sample
    PUBLIC

    INTERFACE

    PRIVATE
        synodic::soul-engine
)

if(WIN32)
    set_target_properties(sample PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

add_subdirectory(src)

