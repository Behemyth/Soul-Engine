
##############################################
#Rasterer

target_sources(${PROJECT_NAME}
    PRIVATE
        Modules/Entity/EntityRenderGraphBackend.cpp
        Modules/Entity/EntityRenderGraphBackend.h
        RenderGraphParameters.cpp
        RenderGraphParameters.h
        RenderGraphBuilder.cpp
        RenderGraphBuilder.h
        RenderGraphModule.cpp
        RenderGraphModule.h
)
