
#Defines a module interface
function(ModuleInterface)

    #Validate the directory structure
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Interface)
        
        file(GLOB_RECURSE INTERFACE_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Interface/*)

        target_sources(${PROJECT_NAME}
            PRIVATE
                ${INTERFACE_SOURCES}
        )

    else()

        message(FATAL_ERROR " An engine module interface must have an \"Interface\" directory.")

    endif()

    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Modules)
        
        #TODO: Do something with Modules

    else()

        message(FATAL_ERROR " An engine module interface must have a \"Modules\" directory.")

    endif()

    #Grab all the subdirectories and add them to the build
    #TODO: Verify `CONFIGURE_DEPENDS` acts as expected on mainline workflows
    file(GLOB MODULE_ITEMS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Modules/*)

    foreach(MODULE_ITEM ${MODULE_ITEMS})
    
        if(IS_DIRECTORY ${MODULE_ITEM})

            add_subdirectory(${MODULE_ITEM})

        else()

            message(FATAL_ERROR " Only module implementations are allowed in the \"Modules\" directory.")

        endif()

    endforeach()

endfunction(ModuleInterface)

#Defines a module implementation
function(ModuleImplementation)

    #Validate the directory structure
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Source)
            
        file(GLOB_RECURSE MODULE_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Source/*)

        target_sources(${PROJECT_NAME}
            PRIVATE
                ${MODULE_SOURCES}
        )

    else()

        message(FATAL_ERROR " An engine module implementation must have a \"Source\" directory.")

    endif()

    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Tests)
        
        #TODO: Do something with Tests

    endif()

    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Resources)

        #TODO: Do something with Resources

    endif()

endfunction(ModuleImplementation)