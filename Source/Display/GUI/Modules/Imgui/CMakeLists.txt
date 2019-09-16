
##############################################
#Imgui GUI

find_package(imgui)


##############################################
#Options

option(BUILD_IMGUI_GUI "Build the Imgui GUI backend" ${imgui_FOUND})


##############################################
#Module

if(${BUILD_IMGUI_GUI})

    ModuleImplementation(
        TARGET IMGUI_MODULE
    )

    target_link_libraries(${IMGUI_MODULE}
        PRIVATE	
            imgui::imgui
    )

endif()