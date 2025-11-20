
##############################################
#Core

# Soul.cpp removed - Soul is now a template in engine.ixx
# target_sources(soul-engine
#     PRIVATE
# 			Soul.cpp
# )

add_subdirectory(composition)
add_subdirectory(frame)
add_subdirectory(geometry)
add_subdirectory(interface)
add_subdirectory(material)
add_subdirectory(object)
add_subdirectory(scene)
add_subdirectory(structure)
add_subdirectory(system)
add_subdirectory(utility)
