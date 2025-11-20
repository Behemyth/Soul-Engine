
##############################################
#Geometry

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			geometry.ixx
			BoundingBox.ixx
			Vertex.ixx
			Face.ixx
			Tet.ixx
    PRIVATE
			BoundingBox.cpp
 "../../render/raster/backend/mock/mock.ixx")
