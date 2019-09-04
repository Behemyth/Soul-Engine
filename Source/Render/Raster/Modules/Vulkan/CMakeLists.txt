
##############################################
#Rasterer

#TODO: Remove reliance on pre-installed VulkanSDK. Via Conan instead.
find_package(Vulkan)

##############################################
#Options

option(BUILD_VULKAN_RASTER "Build the Vulkan Raster backend" ${Vulkan_FOUND})#TODO: Implement use of this option


target_sources(${PROJECT_NAME}
    PRIVATE
        Buffer/VulkanBuffer.h
        Command/VulkanCommandBuffer.cpp
        Command/VulkanCommandBuffer.h
        Command/VulkanCommandPool.cpp
        Command/VulkanCommandPool.h
        Device/VulkanDevice.cpp
        Device/VulkanDevice.h
        Device/VulkanQueue.cpp
        Device/VulkanQueue.h
        Device/VulkanPhysicalDevice.cpp
        Device/VulkanPhysicalDevice.h
        VulkanFramebuffer.cpp
        VulkanFramebuffer.h
        Pipeline/VulkanPipeline.cpp
        Pipeline/VulkanPipeline.h
        Pipeline/VulkanPipelineLayout.cpp
        Pipeline/VulkanPipelineLayout.h
        Pipeline/VulkanPipelineCache.cpp
        Pipeline/VulkanPipelineCache.h
        VulkanInstance.cpp
        VulkanInstance.h
        VulkanRasterBackend.cpp
        VulkanRasterBackend.h
        VulkanRenderPass.cpp
        VulkanRenderPass.h
        VulkanSubPass.cpp
        VulkanSubPass.h
        VulkanSemaphore.cpp
        VulkanSemaphore.h
        VulkanFence.cpp
        VulkanFence.h
        Surface/VulkanFrame.cpp
        Surface/VulkanFrame.h
        VulkanShader.cpp
        VulkanShader.h
        Surface/VulkanSurface.cpp
        Surface/VulkanSurface.h
        Surface/VulkanSwapChain.cpp
        Surface/VulkanSwapChain.h
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE	
        $<$<BOOL:${BUILD_VULKAN_RASTER}>:Vulkan::Vulkan>
)