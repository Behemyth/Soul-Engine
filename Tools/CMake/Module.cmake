
#Defines a module interface
function(ModuleInterface)

    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Interface)
        
        file(GLOB_RECURSE INTERFACE_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Interface/*)

        target_sources(${PROJECT_NAME}
            PRIVATE
                ${INTERFACE_SOURCES}
        )

    else()

        message(FATAL_ERROR "An engine module interface must have an \"Interface\" directory.")

    endif()

    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Modules)
        
         #Grab all the subdirectories and add them to the build
        file(GLOB MODULE_ITEMS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Modules/*)

        foreach(MODULE_ITEM ${MODULE_ITEMS})
        
            if(IS_DIRECTORY ${MODULE_ITEM})

                add_subdirectory(${MODULE_ITEM})

            else()

                message(FATAL_ERROR "Only module implementations are allowed in the \"Modules\" directory.")

            endif()

        endforeach()

    else()

        message(FATAL_ERROR "An engine module interface must have a \"Modules\" directory.")

    endif()

endfunction()


#Defines a module implementation
function(ModuleImplementation)

    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Source)

        file(GLOB_RECURSE MODULE_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Source/*)

        target_sources(${PROJECT_NAME}
            PRIVATE
                ${MODULE_SOURCES}
        )

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


#Sets the input variable to true/false if the given directory is an Engine Module
function(IsModule VAR_NAME DIRECTORY)

    #TODO: Make the IsModule check better than looking for a CMakeLists
    file(GLOB IS_MODULE CONFIGURE_DEPENDS ${DIRECTORY}/CMakeLists.txt)

    if(IS_MODULE)

        set(${VAR_NAME} 1 PARENT_SCOPE)

    else()

        set(${VAR_NAME} 0 PARENT_SCOPE)

    endif()


endfunction()


#Searches the given directory recursively for the first module it comes across.
#The breadth of all subdirectories are searched
function(FindModules VAR_NAME DIRECTORY)

    #Validate the input
    if(NOT IS_DIRECTORY ${DIRECTORY})

        message(FATAL_ERROR "\"${DIRECTORY}\" is not a directory")
    
    endif()


    IsModule(IS_MODULE ${DIRECTORY})

    #If the current directory is a module, append it to the list of modules
    if(${IS_MODULE})

        set(${VAR_NAME} ${DIRECTORY} PARENT_SCOPE)
        return()

    endif()

    #If no module has been found, search for a module recursively 
    file(GLOB SUB_DIRECTORIES CONFIGURE_DEPENDS ${DIRECTORY}/*)

    set(SUB_LIST)
    foreach(SUB_DIRECTORY ${SUB_DIRECTORIES})

        if(IS_DIRECTORY ${SUB_DIRECTORY})
           
            FindModules(INNER_LIST ${SUB_DIRECTORY})

            #Append the inner list into this function's list
            list(APPEND SUB_LIST ${INNER_LIST})

        endif()

    endforeach()

    #Set the parent once
    set(${VAR_NAME} ${SUB_LIST} PARENT_SCOPE)

endfunction()