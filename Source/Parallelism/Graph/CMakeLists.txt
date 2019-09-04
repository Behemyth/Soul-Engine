
##############################################
#Parallelism

target_sources(${PROJECT_NAME}
    PRIVATE
        Modules/EntityGraph/Graph.cpp
        Modules/EntityGraph/Graph.h
        Modules/EntityGraph/GraphNode.cpp
        Modules/EntityGraph/GraphNode.h
        Modules/EntityGraph/GraphTask.cpp
        Modules/EntityGraph/GraphTask.h
        GraphTask.h
        Graph.h
)

target_compile_definitions(${PROJECT_NAME} 
    PRIVATE
        ENTITY_GRAPH=BUILD_ENTITY_GRAPH
)