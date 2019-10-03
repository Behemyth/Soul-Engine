
#Defines a module interface
function(ModuleInterface DIRECTORY)

    IsModuleInterface(IS_MODULE ${DIRECTORY} VALIDATE)

    file(GLOB_RECURSE INTERFACE_SOURCES CONFIGURE_DEPENDS ${DIRECTORY}/Interface/*)

    #Grab the directory information for the module name
    ModuleName(MODULE_NAME ${DIRECTORY})  

    project(${MODULE_NAME}
        LANGUAGES CXX
    )

    add_library(${MODULE_NAME})
    add_library(synodic::${MODULE_NAME} ALIAS ${MODULE_NAME})

    #Set the user target name
    set(${MODULE_INTERFACE_TARGET} ${MODULE_NAME} PARENT_SCOPE)

    set_target_properties(${MODULE_NAME}
        PROPERTIES 
            LINKER_LANGUAGE CXX
            CXX_EXTENSIONS OFF	
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED ON
            USE_FOLDERS ON
    )

    target_include_directories(${MODULE_NAME}
        PUBLIC 
            ${DIRECTORY}/Interface
    )

    target_sources(${MODULE_NAME}
        PRIVATE
            ${INTERFACE_SOURCES}
    )

    #Provides Visual Studio filter support
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${INTERFACE_SOURCES})


endfunction()


#Defines a module implementation
function(ModuleImplementation DIRECTORY)

    #Grab the directory information for the implementation name
    ModuleImplementationName(IMPLEMENTATION_NAME MODULE_NAME ${DIRECTORY})

    #Grab all the subdirectories and add them to the build
    file(GLOB_RECURSE IMPLEMENTATION_SOURCES CONFIGURE_DEPENDS ${DIRECTORY}/Source/*)

    add_library(${IMPLEMENTATION_NAME})
    add_library(synodic::${IMPLEMENTATION_NAME} ALIAS ${IMPLEMENTATION_NAME})

    #Set the user target name
    set(${MODULE_IMPLEMENTATION_TARGET} ${IMPLEMENTATION_NAME} PARENT_SCOPE)

    set_target_properties(${IMPLEMENTATION_NAME}
        PROPERTIES 
            LINKER_LANGUAGE CXX
            CXX_EXTENSIONS OFF	
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED ON
    )

    target_include_directories(${IMPLEMENTATION_NAME}
        PRIVATE 
            ${DIRECTORY}/Source
    )

    target_link_libraries(${IMPLEMENTATION_NAME}
        PRIVATE
            synodic::${MODULE_NAME}
            ${MODULES}
    )

    target_sources(${IMPLEMENTATION_NAME}
        PRIVATE
            ${IMPLEMENTATION_SOURCES}
    )

    #Provides Visual Studio filter support
    source_group(TREE ${DIRECTORY} FILES ${IMPLEMENTATION_SOURCES})



endfunction()

#Sets the input variable to true/false if the given directory is an Engine Module Interface
#If 'VALIDATE' is set an error will be triggered if the directory is not an interface
function(IsModuleInterface VAR_NAME DIRECTORY)

    set(Options VALIDATE)
    set(OneValueArgs)
    set(MultiValueArgs)
    cmake_parse_arguments(IS_MODULE "${Options}" "${OneValueArgs}" "${MultiValueArgs}" ${ARGN})
    
    #Grab the directory information for the module name
    ModuleName(MODULE_NAME ${DIRECTORY})

    if(NOT EXISTS ${DIRECTORY}/README.rst)

        if(${IS_MODULE_VALIDATE})

            message(FATAL_ERROR "${MODULE_NAME}: An engine module interface must have a README.rst")

        endif()

        set(${VAR_NAME} 0 PARENT_SCOPE)
        return()

    endif()

    if(NOT EXISTS ${DIRECTORY}/CMakeLists.txt)

        if(${IS_MODULE_VALIDATE})

            message(FATAL_ERROR "${MODULE_NAME}: An engine module interface must have a CMakeLists.txt")

        endif()

        set(${VAR_NAME} 0 PARENT_SCOPE)
        return()

    endif()

    if(NOT EXISTS ${DIRECTORY}/Interface)

        if(${IS_MODULE_VALIDATE})

            message(FATAL_ERROR "${MODULE_NAME}: An engine module interface must have a \"Interface\" directory.")

        endif()

        set(${VAR_NAME} 0 PARENT_SCOPE)
        return()

    endif()



    if (NOT EXISTS ${DIRECTORY}/Interface/${MODULE_NAME}.h)

        if(${IS_MODULE_VALIDATE})

            message(FATAL_ERROR "${MODULE_NAME}: An engine module interface must have a file named \"${MODULE_NAME}.h\"")

        endif()

        set(${VAR_NAME} 0 PARENT_SCOPE)
        return()

    endif()

    if(NOT EXISTS ${DIRECTORY}/Modules)

        if(${IS_MODULE_VALIDATE})

            message(FATAL_ERROR "${MODULE_NAME}: An engine module interface must have a \"Modules\" directory.")

        endif()

        set(${VAR_NAME} 0 PARENT_SCOPE)
        return()

    endif()

    #Validate the Modules directory
    file(GLOB MODULE_ITEMS CONFIGURE_DEPENDS ${DIRECTORY}/Modules/*)

    foreach(MODULE_ITEM ${MODULE_ITEMS})
    
        if(NOT IS_DIRECTORY ${MODULE_ITEM})

            if(${IS_MODULE_VALIDATE})

                message(FATAL_ERROR "${MODULE_NAME}: Only module implementations are allowed in the \"Modules\" directory.")

            endif()
            
            set(${VAR_NAME} 0 PARENT_SCOPE)
            return()

        endif()

        if(${IS_MODULE_VALIDATE})

            IsModuleImplementation(IS_IMPLEMENTATION ${MODULE_ITEM} VALIDATE)

        else()

            IsModuleImplementation(IS_IMPLEMENTATION ${MODULE_ITEM})

            if(NOT ${IS_IMPLEMENTATION})
                
                set(${VAR_NAME} 0 PARENT_SCOPE)
                return()

            endif()

        endif()

    endforeach()

    #Everything has been validated, return true
    set(${VAR_NAME} 1 PARENT_SCOPE)

endfunction()

#Sets the input variable to true/false if the given directory is an Engine Module Interface
#If 'VALIDATE' is set an error will be triggered if the directory is not an implementation
function(IsModuleImplementation VAR_NAME DIRECTORY)

    set(Options VALIDATE)
    set(OneValueArgs)
    set(MultiValueArgs)
    cmake_parse_arguments(IS_MODULE "${Options}" "${OneValueArgs}" "${MultiValueArgs}" ${ARGN})

    ModuleImplementationName(IMPLEMENTATION_NAME MODULE_NAME ${DIRECTORY})

    if(NOT EXISTS ${DIRECTORY}/README.rst)

        if(${IS_MODULE_VALIDATE})

            message(FATAL_ERROR "${IMPLEMENTATION_NAME}: An engine module implementation must have a README.rst")

        endif()

        set(${VAR_NAME} 0 PARENT_SCOPE)
        return()

    endif()

    if(NOT EXISTS ${DIRECTORY}/CMakeLists.txt)

        if(${IS_MODULE_VALIDATE})

            message(FATAL_ERROR "${IMPLEMENTATION_NAME}: An engine module implementation must have a CMakeLists.txt")

        endif()

        set(${VAR_NAME} 0 PARENT_SCOPE)
        return()

    endif()

    if (NOT EXISTS ${DIRECTORY}/Source)
        
        if(${IS_MODULE_VALIDATE})

            message(FATAL_ERROR "${IMPLEMENTATION_NAME}: An engine module implementation must have a \"Source\" directory")

        endif()

        set(${VAR_NAME} 0 PARENT_SCOPE)
        return()

    endif()

    if (NOT EXISTS ${DIRECTORY}/Tests)
        
        if(${IS_MODULE_VALIDATE})

            message(FATAL_ERROR "${IMPLEMENTATION_NAME}: An engine module implementation must have a \"Tests\" directory")

        endif()

        set(${VAR_NAME} 0 PARENT_SCOPE)
        return()

    endif()

    if (NOT EXISTS ${DIRECTORY}/Tests/README.rst)
        
        if(${IS_MODULE_VALIDATE})

            message(FATAL_ERROR "${IMPLEMENTATION_NAME}: An engine module implementation must have a \"README.rst\" inside the \"Tests\" directory")

        endif()

        set(${VAR_NAME} 0 PARENT_SCOPE)
        return()

    endif()

    if (NOT EXISTS ${DIRECTORY}/Source/${IMPLEMENTATION_NAME}.h)

        if(${IS_MODULE_VALIDATE})

            message(FATAL_ERROR "${IMPLEMENTATION_NAME}: An engine module implementation must have a file named \"${IMPLEMENTATION_NAME}.h\"")

        endif()

        set(${VAR_NAME} 0 PARENT_SCOPE)
        return()

    endif()

    #Everything has been validated, return true
    set(${VAR_NAME} 1 PARENT_SCOPE)

endfunction()

function(ModuleName NAME_VAR DIRECTORY)

    get_filename_component(MODULE_NAME ${DIRECTORY} NAME)
    set(${NAME_VAR} ${MODULE_NAME}Module PARENT_SCOPE)

endfunction()


function(ModuleImplementationName NAME_VAR MODULE_NAME_VAR DIRECTORY)

    get_filename_component(MODULE_DIRECTORY ${DIRECTORY}/../.. ABSOLUTE)
    get_filename_component(MODULE_NAME ${MODULE_DIRECTORY} NAME)
    get_filename_component(IMPLEMENTATION_NAME ${DIRECTORY} NAME)

    set(${MODULE_NAME_VAR} ${MODULE_NAME}Module PARENT_SCOPE)
    set(${NAME_VAR} ${IMPLEMENTATION_NAME}${MODULE_NAME}Backend PARENT_SCOPE)

endfunction()

#An extension of `add_subdirectory`
function(ProcessModules PARENT_TARGET DIRECTORY)

    file(GLOB MODULES CONFIGURE_DEPENDS ${DIRECTORY}/*)

    foreach(MODULE ${MODULES})


        if(IS_DIRECTORY ${MODULE})

            if(EXISTS ${MODULE}/CMakeLists.txt)

                ModuleInterface(${MODULE})

            else()

                ProcessModules(${PARENT_TARGET} ${MODULE})

            endif()

        else()

            message(FATAL_ERROR "The item \"${MODULE}\" must belong to a module")

        endif()    

    endforeach()

endfunction()