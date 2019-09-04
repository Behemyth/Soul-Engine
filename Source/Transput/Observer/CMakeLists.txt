
##############################################
#Resource

target_sources(${PROJECT_NAME}
    PRIVATE
        Modules/Windows/WindowsObserverModule.cpp
        Modules/Windows/WindowsObserverModule.h
        Modules/Poll/PollObserverModule.cpp
        Modules/Poll/PollObserverModule.h
        ObserverModule.cpp
        ObserverModule.h
)
