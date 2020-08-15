cmake_minimum_required(VERSION 3.17.2 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}) #TODO: remove with new Conan generator
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}) #TODO: remove with new Conan generator

# Global Options
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON) # Opposite module/PCF order for `find_package`
set_property(GLOBAL PROPERTY USE_FOLDERS ON) # Group the default targets in a folder


project(tests 
	DESCRIPTION "Test project for ${CMAKE_PROJECT_NAME}"
	LANGUAGES CXX
)


add_executable(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME}
	PROPERTIES 
		LINKER_LANGUAGE CXX
		CXX_STANDARD 20
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS NO
)

find_package(doctest 2.4.0 REQUIRED)

target_link_libraries(${PROJECT_NAME}
PUBLIC
	${CMAKE_PROJECT_NAME}
    doctest::doctest
)


file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*)

target_sources(${PROJECT_NAME}
	PRIVATE
		${SOURCES}
)

doctest_discover_tests(tests)