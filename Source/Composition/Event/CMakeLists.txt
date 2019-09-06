
##############################################
#Composition/Event

include(Module.cmake)

target_sources(${PROJECT_NAME}
    PRIVATE
        Event.h
        EventRegistry.cpp
        EventRegistry.h
)
