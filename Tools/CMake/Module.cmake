
#Defines a module interface
function(ModuleInterface)

    #Parse the function arguments
    set(Options)
    set(OneValueArgs TARGET)
    set(MultiValueArgs)
    cmake_parse_arguments(MODULE_INTERFACE "${Options}" "${OneValueArgs}" "${MultiValueArgs}" ${ARGN})

    #Grab the directory information for the module name
    get_filename_component(ModulePath "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
    get_filename_component(ModuleName "${ModulePath}" NAME)

    set(COMBINED_NAME ${ModuleName}Module)

    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Interface)
        
        if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Interface/${ModuleName}Module.h)
            
            message(FATAL_ERROR "An engine module interface must have a file named \"${ModuleName}Module.h\"")

        endif()

        file(GLOB_RECURSE INTERFACE_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Interface/*)

        project(${COMBINED_NAME}
            LANGUAGES CXX
        )

        add_library(${COMBINED_NAME} "")
        add_library(synodic::${COMBINED_NAME} ALIAS ${COMBINED_NAME})

        #Set the user target name
        set(${MODULE_INTERFACE_TARGET} ${COMBINED_NAME} PARENT_SCOPE)

        set_target_properties(${COMBINED_NAME}
            PROPERTIES 
                LINKER_LANGUAGE CXX
                CXX_EXTENSIONS OFF	
                CXX_STANDARD 20
                CXX_STANDARD_REQUIRED ON
                USE_FOLDERS ON
        )

        target_include_directories(${COMBINED_NAME}
            PUBLIC 
                ${ModulePath}/Interface
        )

        target_sources(${COMBINED_NAME}
            PRIVATE
                ${INTERFACE_SOURCES}
        )

        #Provides Visual Studio filter support
        source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${INTERFACE_SOURCES})

    else()

        message(FATAL_ERROR "An engine module interface must have an \"Interface\" directory.")

    endif()

    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Modules)
        
        #Grab all the subdirectories and add them to the build
        file(GLOB MODULE_ITEMS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Modules/*)

        foreach(MODULE_ITEM ${MODULE_ITEMS})
        
            if(NOT IS_DIRECTORY ${MODULE_ITEM})

                message(FATAL_ERROR "Only module implementations are allowed in the \"Modules\" directory.")

            endif()

        endforeach()

    else()

        message(FATAL_ERROR "An engine module interface must have a \"Modules\" directory.")

    endif()

endfunction()


#Defines a module implementation
function(ModuleImplementation)

    #Parse the function arguments
    set(Options)
    set(OneValueArgs TARGET)
    set(MultiValueArgs)
    cmake_parse_arguments(MODULE_IMPLEMENTATION "${Options}" "${OneValueArgs}" "${MultiValueArgs}" ${ARGN})

    #Grab the directory information for the module name
    get_filename_component(ModulePath "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
    get_filename_component(ModuleName "${ModulePath}" NAME)

    #Grab the directory information for the implementation name
    get_filename_component(ImplementationPath "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
    get_filename_component(ImplementationName "${ImplementationPath}" NAME)

    set(MODULE_COMBINED_NAME ${ModuleName}Module)
    set(COMBINED_NAME ${ImplementationName}${ModuleName}Backend)

    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Source)

        if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Source/${COMBINED_NAME}.h)

            message(FATAL_ERROR "An engine module implementation must have a file named \"${COMBINED_NAME}.h\"")

        endif()

        #Grab all the subdirectories and add them to the build
        file(GLOB_RECURSE IMPLEMENTATION_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Source/*)

        add_library(${COMBINED_NAME} "")
        add_library(synodic::${COMBINED_NAME} ALIAS ${COMBINED_NAME})

        #Set the user target name
        set(${MODULE_IMPLEMENTATION_TARGET} ${COMBINED_NAME} PARENT_SCOPE)

        set_target_properties(${COMBINED_NAME}
            PROPERTIES 
                LINKER_LANGUAGE CXX
                CXX_EXTENSIONS OFF	
                CXX_STANDARD 20
                CXX_STANDARD_REQUIRED ON
        )

        target_include_directories(${COMBINED_NAME}
            PRIVATE 
                ${CMAKE_CURRENT_SOURCE_DIR}/Source
        )

        target_link_libraries(${COMBINED_NAME}
            PRIVATE
                synodic::${MODULE_COMBINED_NAME}
                ${MODULES}
        )

        target_sources(${COMBINED_NAME}
            PRIVATE
                ${IMPLEMENTATION_SOURCES}
        )

        #Link to the module library
        target_link_libraries(${COMBINED_NAME}
            PRIVATE
                synodic::${MODULE_COMBINED_NAME}
        )

        #Provides Visual Studio filter support
        source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${IMPLEMENTATION_SOURCES})

    else()

        message(FATAL_ERROR "An engine module implementation must have a \"Source\" directory.")

    endif()

    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Tests)
        
        #TODO: Do something with Tests

    endif()

    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Resources)

        #TODO: Do something with Resources

    endif()

endfunction()