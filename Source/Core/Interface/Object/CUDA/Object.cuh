#pragma once

//#include "Core/Material/Material.h"
//#include "Core/Geometry/Vertex.h"
//#include "Core/Geometry/Face.h"
//#include "Core/Geometry/Tet.h"
#include "Core/Geometry/BoundingBox.h"
//#include "Types.h"
//
//#include <vector>

class Object {

public:

	Object() = default;
	//Object(std::string, Material);
	//~Object();

	//bool requestRemoval;
	//bool ready;
	//bool isStatic;

	//void AddVertices(Vertex*, uint);
	//void AddFaces(Face*, uint);
	//void ExtractFromFile(const char*);

	//uint verticeAmount;
	//uint faceAmount;
	//uint tetAmount;
	//uint materialAmount;

	//std::vector<Vertex> vertices;
	//std::vector<Face> faces;
	//std::vector<Tet> tets;
	//std::vector<Material> materials;

	BoundingBox box; //in object space

protected:

private:


};
