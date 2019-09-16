
##############################################
#GLFW Window

find_package(glfw)

##############################################
#Options

option(BUILD_GLFW_DISPLAY "Build the GLFW Display backend" ${glfw_FOUND})#TODO: Implement use of this option


##############################################
#Module

if(${BUILD_GLFW_DISPLAY})

    ModuleImplementation(
        TARGET GLFW_MODULE
    )

    target_link_libraries(${GLFW_MODULE}
        PRIVATE	
            glfw::glfw
    )

endif()