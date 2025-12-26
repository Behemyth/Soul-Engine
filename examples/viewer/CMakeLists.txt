cmake_minimum_required(VERSION 4.0 FATAL_ERROR)

# Enable std module support for MSVC - MUST be before project()
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")

set(CMAKE_CXX_MODULE_STD ON)

project(Viewer)

add_executable(viewer)

set_target_properties(viewer
	PROPERTIES
        LINKER_LANGUAGE CXX
	   	CXX_EXTENSIONS OFF
        CXX_STANDARD 23
)

target_link_libraries(viewer
    PUBLIC

    INTERFACE

    PRIVATE
        synodic::soul-engine
)

# Ensure we're linking the C++ standard library
if(MSVC)
    target_compile_options(viewer PRIVATE /MDd)
endif()

if(WIN32)
    # Use WINDOWS subsystem but keep standard C++ entry point
    set_target_properties(viewer PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

add_subdirectory(src)

