
##############################################
#Window

option(BUILD_GLFW_DISPLAY "Build the GLFW Display backend" ON)#TODO: Implement use of this option

target_sources(${PROJECT_NAME}
    PRIVATE
        GLFWWindowBackend.cpp
        GLFWWindowBackend.h
        GLFWWindow.cpp
        GLFWWindow.h
	GLFWMonitor.cpp
	GLFWMonitor.h
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE	
        $<$<BOOL:${BUILD_GLFW_DISPLAY}>:CONAN_PKG::glfw
)