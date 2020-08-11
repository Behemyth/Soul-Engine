cmake_minimum_required(VERSION 3.17.2 FATAL_ERROR)

##############################################
#Project 

project(SoulInfo)


##############################################
#Dependancies


##############################################
#Sources

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*)


##############################################
#Targets

add_executable(${PROJECT_NAME} ${SOURCES})

set_target_properties(${PROJECT_NAME}
PROPERTIES 
	LINKER_LANGUAGE CXX
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS NO
)

target_include_directories(${PROJECT_NAME}
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
    PRIVATE

)

target_link_libraries(${PROJECT_NAME}
    PUBLIC

    INTERFACE

    PRIVATE	
        synodic::soul-engine
)