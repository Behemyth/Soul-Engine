//#include "Object.cuh"
//#include "Parallelism/ComputeOld/CUDA/Utility/CUDAHelper.cuh"
//
////#define TINYOBJLOADER_IMPLEMENTATION
////#include <tiny_obj_loader.h>
//
//#include <unordered_map>
//
//Object::Object() {
//
//	verticeAmount = 0;
//	faceAmount = 0;
//	tetAmount = 0;
//	materialAmount = 0;
//	ready = false;
//	requestRemoval = false;
//	isStatic = false;
//
//}
//Object::Object(std::string name, Material mat) {
//
//	verticeAmount = 0;
//	faceAmount = 0;
//	tetAmount = 0;
//	materialAmount = 0;
//	ready = false;
//	requestRemoval = false;
//	isStatic = false;
//
//	materials.push_back(mat);
//	materialAmount++;
//
//	ExtractFromFile(name.c_str());
//}
//
//Object::~Object() {
//
//}
//
//void Object::AddVertices(Vertex* verticesIn, uint vSize) {
//
//}
//void Object::AddFaces(Face* facesIn, uint fSize) {
//
//}
//void Object::ExtractFromFile(const char* name) {
//
//	/*tinyobj::attrib_t attrib;
//	std::vector<tinyobj::shape_t> shapes;
//	std::vector<tinyobj::material_t> materialsT;
//	std::string err;
//
//	if (!tinyobj::LoadObj(&attrib, &shapes, &materialsT, &err, name)) {
//		throw std::runtime_error(err);
//	}
//
//	assert(shapes.size() == 1);
//
//	faceAmount = static_cast<uint>(shapes[0].mesh.indices.size() / 3);
//
//	glm::vec3 max = glm::vec3(attrib.vertices[0], attrib.vertices[1], attrib.vertices[2]);
//	glm::vec3 min = max;
//
//	std::vector<uint> facesUngrouped;
//
//	std::unordered_map<Vertex, uint> uniqueVertices = {};
//
//	for (const auto& shape : shapes) {
//		for (const auto& index : shape.mesh.indices) {
//			Vertex vertex;
//
//			vertex.position = {
//				attrib.vertices[3 * index.vertex_index + 0],
//				attrib.vertices[3 * index.vertex_index + 1],
//				attrib.vertices[3 * index.vertex_index + 2]
//			};
//
//			vertex.textureCoord = {
//				attrib.texcoords[2 * index.texcoord_index + 0],
//				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
//			};
//
//			vertex.normal = {
//				attrib.normals[3 * index.normal_index + 0],
//				attrib.normals[3 * index.normal_index + 1],
//				attrib.normals[3 * index.normal_index + 2]
//			};
//
//			if (uniqueVertices.count(vertex) == 0) {
//				uniqueVertices[vertex] = static_cast<uint>(vertices.size());
//
//				max = glm::max(vertex.position, max);
//				min = glm::min(vertex.position, min);
//
//				vertices.push_back(vertex);
//			}
//
//			facesUngrouped.push_back(uniqueVertices[vertex]);
//		}
//	}
//
//	verticeAmount = static_cast<uint>(vertices.size());
//	faces.resize(faceAmount);
//	for (uint i = 0; i < facesUngrouped.size() / 3; i++) {
//		faces[i].indices.x = facesUngrouped[i * 3 + 0];
//		faces[i].indices.y = facesUngrouped[i * 3 + 1];
//		faces[i].indices.z = facesUngrouped[i * 3 + 2];
//		faces[i].material = 0;
//	}*/
//
//	//const auto& shape = shapes[0];
//
//	//for (uint f = 0; f < faceAmount; f++) {
//
//
//	//	//grab commenly used variables
//	//	tinyobj::index_t id0 = shape.mesh.indices[3 * f + 0];
//	//	tinyobj::index_t id1 = shape.mesh.indices[3 * f + 1];
//	//	tinyobj::index_t id2 = shape.mesh.indices[3 * f + 2];
//
//	//	int current_material_id = shape.mesh.material_ids[f];
//
//	//	faces[f].indices.x = id0.vertex_index;
//	//	faces[f].indices.y = id1.vertex_index;
//	//	faces[f].indices.z = id2.vertex_index;
//
//	//	///////////////////
//
//	//	vertices[id0.vertex_index].position.x = attrib.vertices[id0.vertex_index * 3 + 0];
//	//	vertices[id0.vertex_index].position.y = attrib.vertices[id0.vertex_index * 3 + 1];
//	//	vertices[id0.vertex_index].position.z = attrib.vertices[id0.vertex_index * 3 + 2];
//
//	//	vertices[id0.vertex_index].textureCoord.x = attrib.texcoords[id0.texcoord_index * 2 + 0];
//	//	vertices[id0.vertex_index].textureCoord.y = 1.0f - attrib.texcoords[id0.texcoord_index * 2 + 1];
//
//	//	vertices[id0.vertex_index].normal.x = attrib.normals[id0.normal_index * 3 + 0];
//	//	vertices[id0.vertex_index].normal.y = attrib.normals[id0.normal_index * 3 + 1];
//	//	vertices[id0.vertex_index].normal.z = attrib.normals[id0.normal_index * 3 + 2];
//
//	//	max = glm::max(vertices[id0.vertex_index].position, max);
//	//	min = glm::min(vertices[id0.vertex_index].position, min);
//
//	//	///////////////////
//
//	//	vertices[id1.vertex_index].position.x = attrib.vertices[id1.vertex_index * 3 + 0];
//	//	vertices[id1.vertex_index].position.y = attrib.vertices[id1.vertex_index * 3 + 1];
//	//	vertices[id1.vertex_index].position.z = attrib.vertices[id1.vertex_index * 3 + 2];
//
//	//	vertices[id1.vertex_index].textureCoord.x = attrib.texcoords[id1.texcoord_index * 2 + 0];
//	//	vertices[id1.vertex_index].textureCoord.y = 1.0f - attrib.texcoords[id1.texcoord_index * 2 + 1];
//
//	//	vertices[id1.vertex_index].normal.x = attrib.normals[id1.normal_index * 3 + 0];
//	//	vertices[id1.vertex_index].normal.y = attrib.normals[id1.normal_index * 3 + 1];
//	//	vertices[id1.vertex_index].normal.z = attrib.normals[id1.normal_index * 3 + 2];
//
//	//	max = glm::max(vertices[id1.vertex_index].position, max);
//	//	min = glm::min(vertices[id1.vertex_index].position, min);
//
//	//	///////////////////
//
//	//	vertices[id2.vertex_index].position.x = attrib.vertices[id2.vertex_index * 3 + 0];
//	//	vertices[id2.vertex_index].position.y = attrib.vertices[id2.vertex_index * 3 + 1];
//	//	vertices[id2.vertex_index].position.z = attrib.vertices[id2.vertex_index * 3 + 2];
//
//	//	vertices[id2.vertex_index].textureCoord.x = attrib.texcoords[id2.texcoord_index * 2 + 0];
//	//	vertices[id2.vertex_index].textureCoord.y = 1.0f - attrib.texcoords[id2.texcoord_index * 2 + 1];
//
//	//	vertices[id2.vertex_index].normal.x = attrib.normals[id2.normal_index * 3 + 0];
//	//	vertices[id2.vertex_index].normal.y = attrib.normals[id2.normal_index * 3 + 1];
//	//	vertices[id2.vertex_index].normal.z = attrib.normals[id2.normal_index * 3 + 2];
//
//	//	max = glm::max(vertices[id2.vertex_index].position, max);
//	//	min = glm::min(vertices[id2.vertex_index].position, min);
//
//	//	faces[f].material = 0;
//	//}
//
//	//LOG(TRACE, "/nINDICES: " << faceAmount << std::endl;
//	//for (int i = 0; i < faceAmount; i++){
//	//	LOG(TRACE,("%i ", faces[i].indices.x);
//	//	LOG(TRACE,("%i ", faces[i].indices.y);
//	//	LOG(TRACE,("%i /n", faces[i].indices.z);
//	//}
//
//	//LOG(TRACE, "/nVERTICES: " << verticeAmount << std::endl;
//
//	//for (int i = 0; i < verticeAmount; i++){
//	//	LOG(TRACE, "/n	Positions: "  << std::endl;
//
//	//	LOG(TRACE,("%f ", vertices[i].position.x);
//	//	LOG(TRACE,("%f ", vertices[i].position.y);
//	//	LOG(TRACE,("%f /n", vertices[i].position.z);
//
//	//	LOG(TRACE,"/n	Normals: " << std::endl;
//
//	//	LOG(TRACE,("%f ", vertices[i].normal.x);
//	//	LOG(TRACE,("%f ", vertices[i].normal.y);
//	//	LOG(TRACE,("%f /n", vertices[i].normal.z);
//
//	//	LOG(TRACE, "/n	TexCoords: " << std::endl;
//
//	//	LOG(TRACE,("%f ", vertices[i].textureCoord.x);
//	//	LOG(TRACE,("%f /n", vertices[i].textureCoord.y);
//	//}
//
//	//box.max = max;
//	//box.min = min;
//
//}