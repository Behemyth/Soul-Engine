
##############################################
#Parallelism

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			parallelism.graph.ixx
			modules/entity_graph/GraphNode.ixx
			modules/entity_graph/GraphTask.ixx
			modules/entity_graph/Graph.ixx
    PRIVATE
		modules/entity_graph/GraphTask.cpp
		modules/entity_graph/Graph.cpp
)

target_compile_definitions(soul-engine
    PRIVATE
        ENTITY_GRAPH=BUILD_ENTITY_GRAPH
)
