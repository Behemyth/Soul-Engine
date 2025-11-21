cmake_minimum_required(VERSION 4.0 FATAL_ERROR)

# Enable std module support for MSVC - MUST be before project()
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")

set(CMAKE_CXX_MODULE_STD ON)

#Set CMake tools location
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/tool/CMake)


##############################################
#Project

project(SoulEngine
	VERSION 0.0.1
	DESCRIPTION "Real-time simulation and rendering engine."
	LANGUAGES CXX
)

##############################################
#Dependencies

find_package(honesty REQUIRED)
find_package(synodic-template-library REQUIRED)
find_package(cppaste REQUIRED)
find_package(SDL3 REQUIRED)
find_package(VulkanMemoryAllocator REQUIRED)
find_package(Vulkan REQUIRED)

# Slang is bundled with Vulkan SDK but doesn't provide CMake config
# Set up manually using Vulkan SDK paths
if(NOT TARGET slang::slang)
	# Get Vulkan SDK root by going up from include directory
	get_filename_component(VULKAN_SDK_ROOT "${Vulkan_INCLUDE_DIR}" DIRECTORY)

	find_path(SLANG_INCLUDE_DIR
		NAMES slang.h
		PATHS "${VULKAN_SDK_ROOT}/Include/slang"
		NO_DEFAULT_PATH
	)

	find_library(SLANG_LIBRARY
		NAMES slang
		PATHS "${VULKAN_SDK_ROOT}/Lib"
		NO_DEFAULT_PATH
	)

	if(SLANG_INCLUDE_DIR AND SLANG_LIBRARY)
		add_library(slang::slang UNKNOWN IMPORTED)
		set_target_properties(slang::slang PROPERTIES
			IMPORTED_LOCATION "${SLANG_LIBRARY}"
			INTERFACE_INCLUDE_DIRECTORIES "${SLANG_INCLUDE_DIR}"
		)
		message(STATUS "Found Slang: ${SLANG_LIBRARY}")
	else()
		message(FATAL_ERROR "Slang not found in Vulkan SDK at ${VULKAN_SDK_ROOT}")
	endif()
endif()

##############################################
#Options

option(BUILD_TESTING "Build the tests verifying functionality" ${PROJECT_IS_TOP_LEVEL})
option(BUILD_EXAMPLES "Build example projects" ${PROJECT_IS_TOP_LEVEL})

option(BUILD_SHARED_LIBS "Build shared library" OFF)#TODO: Implement use of this option
option(SOUL_ENABLE_SHADER_HOT_RELOAD "Enable shader hot-reload support with file watching" ${PROJECT_IS_TOP_LEVEL})
option(BUILD_CPU_COMPUTE "Build the OpenCL Compute backend" ON)#TODO: Implement use of this option
option(BUILD_CUDA_COMPUTE "Build the CUDA Compute backend" ${CUDA_FOUND})#TODO: Implement use of this option
option(BUILD_OPENCL_COMPUTE "Build the OpenCL Compute backend" ${OpenCL_FOUND})#TODO: Implement use of this option
option(BUILD_VULKAN_RASTER "Build the Vulkan Raster backend" ON)#TODO: Implement use of this option
option(BUILD_FIBER_SCHEDULER "Build the Fiber Scheduler backend" OFF)
option(BUILD_ENTITY_GRAPH "Build the Entity Graph backend" ON)

##############################################
#Project Target

add_library(VulkanHppModule)
target_sources(VulkanHppModule PUBLIC
  FILE_SET CXX_MODULES
  BASE_DIRS ${Vulkan_INCLUDE_DIR}
  FILES ${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm
)
target_compile_features(VulkanHppModule PUBLIC cxx_std_23)
target_link_libraries(VulkanHppModule PUBLIC Vulkan::Vulkan)

add_library(soul-engine)
add_library(synodic::soul-engine ALIAS soul-engine)

set_target_properties(soul-engine
	PROPERTIES
        LINKER_LANGUAGE CXX
	   	CXX_EXTENSIONS OFF
		CUDA_SEPARABLE_COMPILATION ON
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
		USE_FOLDERS ON
)

target_compile_features(soul-engine PUBLIC cxx_std_23)

target_link_libraries(soul-engine
    PUBLIC
        synodic::cppaste
    PRIVATE
		synodic::honesty
		synodic::synodic-template-library
		SDL3::SDL3
		GPUOpen::VulkanMemoryAllocator
		VulkanHppModule
		slang::slang
)

# Conditionally define SOUL_ENABLE_SHADER_HOT_RELOAD macro
if(SOUL_ENABLE_SHADER_HOT_RELOAD)
	target_compile_definitions(soul-engine PRIVATE SOUL_ENABLE_SHADER_HOT_RELOAD)
endif()

# Create file set for C++20 modules
target_sources(soul-engine
	PUBLIC
		FILE_SET CXX_MODULES FILES
)

##############################################
#Sources

add_subdirectory(
	src
)

# NOTE: For IDE setups that do not use CMake directly, we setup all IDE directories to mimic
# their system directory structure. See https://gitlab.kitware.com/cmake/cmake/-/issues/25992
file(GLOB_RECURSE file_list ${CMAKE_CURRENT_SOURCE_DIR}/src/*)

# Must be called from the target definition directory. Only matched against 'target_sources'
# files (No nested CMakeList.txt, docs, etc).
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${file_list})

##############################################
#Sub-projects

if(BUILD_TESTING AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
	# Do not include CTest because it adds targets and integrations we do not want to support
	enable_testing()

    add_subdirectory(tests)

endif()

if(BUILD_EXAMPLES AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples)

    add_subdirectory(examples)

endif()

# Installation
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Generate package config files
write_basic_package_version_file(
	soul-engineConfigVersion.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
	tool/cmake/soul-engineConfig.cmake.in
	soul-engineConfig.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/soul-engine
)

install(TARGETS soul-engine VulkanHppModule ${VULKAN_MODULE_EXPORT_TARGET}
	EXPORT soul-engineTargets
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}

	FILE_SET CXX_MODULES DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/soul-engine
)

# Create empty include directory to satisfy CMake's exported target requirements
# (module-only library with no headers, but CMake expects the directory to exist)
install(DIRECTORY DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Export and package configuration
install(EXPORT soul-engineTargets
	FILE soul-engineTargets.cmake
	NAMESPACE synodic::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/soul-engine
)

install(
	FILES
	${CMAKE_CURRENT_BINARY_DIR}/soul-engineConfig.cmake
	${CMAKE_CURRENT_BINARY_DIR}/soul-engineConfigVersion.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/soul-engine
)
