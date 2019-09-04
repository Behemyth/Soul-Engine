
##############################################
#GUI

option(BUILD_IMGUI_GUI "Build the Imgui GUI backend" ON)#TODO: Implement use of this option


target_sources(${PROJECT_NAME}
    PRIVATE
        EmptyWidget.h
        ImguiBackend.cpp
        ImguiBackend.h
        ImguiConfig.h
        ImguiLayout.cpp
        ImguiLayout.h
        ImguiWidget.cpp
        ImguiWidget.h
        RenderWidget.h
        SingleLayout.h
)
