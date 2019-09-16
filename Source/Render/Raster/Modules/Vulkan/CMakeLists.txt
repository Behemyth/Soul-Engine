
##############################################
#Vulkan Rasterer

find_package(Vulkan)


##############################################
#Options

option(BUILD_VULKAN_RASTER "Build the Vulkan Raster backend" ${Vulkan_FOUND})


##############################################
#Module

if(${BUILD_VULKAN_RASTER})

    ModuleImplementation()

    target_link_libraries(${PROJECT_NAME}
        PRIVATE	
            Vulkan::Vulkan
    )

endif()