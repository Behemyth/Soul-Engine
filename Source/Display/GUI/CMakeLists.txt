
##############################################
#GUI

target_sources(${PROJECT_NAME}
    PRIVATE
        GUIModule.cpp
        GUIModule.h
        Layout.h
        Modules/Imgui/EmptyWidget.h
        Modules/Imgui/ImguiBackend.cpp
        Modules/Imgui/ImguiBackend.h
        Modules/Imgui/ImguiConfig.h
        Modules/Imgui/ImguiLayout.cpp
        Modules/Imgui/ImguiLayout.h
        Modules/Imgui/ImguiWidget.cpp
        Modules/Imgui/ImguiWidget.h
        Modules/Imgui/RenderWidget.h
        Modules/Imgui/SingleLayout.h
        Widget.h
)
