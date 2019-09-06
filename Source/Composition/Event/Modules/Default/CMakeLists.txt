
##############################################
#Composition/Event

include(Module.cmake)

target_sources(${PROJECT_NAME}
    PRIVATE
        DefaultEvent.h
        DefaultEventRegistry.cpp
        DefaultEventRegistry.h
)
