
##############################################
#Window

target_sources(${PROJECT_NAME}
    PRIVATE
        WindowModule.cpp
        WindowModule.h
        Modules/GLFW/GLFWWindowBackend.cpp
        Modules/GLFW/GLFWWindowBackend.h
        Modules/GLFW/GLFWWindow.cpp
        Modules/GLFW/GLFWWindow.h
	Modules/GLFW/GLFWMonitor.cpp
	Modules/GLFW/GLFWMonitor.h
        Window.cpp
        Window.h
	Monitor.h
)
