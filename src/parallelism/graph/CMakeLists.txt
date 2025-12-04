
##############################################
#Parallelism

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			parallelism.graph.ixx
			modules/entity_graph/graph_node.ixx
			modules/entity_graph/graph_task.ixx
			modules/entity_graph/graph.ixx
)

target_compile_definitions(soul-engine
    PRIVATE
        ENTITY_GRAPH=BUILD_ENTITY_GRAPH
)
