#include "Scene.h"
//
// #define RAY_BIAS_DISTANCE 0.0002f 
// #define BVH_STACK_SIZE 64
// #define DYNAMIC_FETCH_THRESHOLD 20          // If fewer than this active, fetch new rays
//
// #include "Core/Utility/Log/Logger.h"
//
// #include "Core/Scene/Morton Code/MortonCode.h"
//
// #include "Parallelism/ComputeOld/ComputeManager.h"
// #include "Parallelism/ComputeOld/Algorithm/Sort/Sort.h"
//
// Scene::Scene():
// 	//BVH(S_BEST_DEVICE),
// 	sky(S_BEST_DEVICE),
// 	faces(S_BEST_DEVICE),
// 	vertices(S_BEST_DEVICE),
// 	tets(S_BEST_DEVICE),
// 	materials(S_BEST_DEVICE),
// 	//objects(S_BEST_DEVICE),
// 	mortonCodes(S_BEST_DEVICE),
// 	boxes(S_BEST_DEVICE)
// {
//
// 	//BVH.Resize(1);
// 	sky.PushBack({ "Starmap.png" });
//
// 	sky.TransferToDevice();
// }
//
// void Scene::Build(double deltaTime) {
//
// 	Compile();
//
// 	auto size = faces.SizeHost();
// 	if (size > 0) {
//
// 		ComputeDevice device = S_BEST_DEVICE;
//
// 		const auto blockSize = 64;
// 		const GPUExecutePolicy normalPolicy(glm::vec3((size + blockSize - 1) / blockSize, 1, 1), glm::vec3(blockSize, 1, 1), 0, 0);
// 		
// 		//device.Launch(normalPolicy, MortonCode::ComputeGPUFace64, size, mortonCodes.DataDevice(), faces.DataDevice(), vertices.DataDevice());
//
// //		Sort::Sort(mortonCodes, faces);
//
// 	}
//
// 	//bvh.Build(size, BVH, mortonCodes, boxes);
//
// }
//
// void Scene::Compile() {
//
// 	//upload the data
// 	vertices.TransferToDevice();
// 	faces.TransferToDevice();
// 	tets.TransferToDevice();
// 	materials.TransferToDevice();
// 	//objects.TransferToDevice();
// 	boxes.TransferToDevice();
// }
//
// //object pointer is host
// void Scene::AddObject(Object& obj) {
//
// 	//auto faceAmount = faces.SizeHost();
// 	//auto vertexAmount = vertices.SizeHost();
// 	//auto tetAmount = tets.SizeHost();
// 	//auto materialAmount = materials.SizeHost();
// 	////auto objectAmount = objects.SizeHost();
//
// 	//const auto faceOffset = faceAmount;
// 	//const auto vertexOffset = vertexAmount;
// 	//const auto tetOffset = tetAmount;
// 	//const auto materialOffset = materialAmount;
// 	////const auto objectOffset = objectAmount;
//
// 	//tetAmount += obj.tetAmount;
// 	//faceAmount += obj.faceAmount;
// 	//vertexAmount += obj.verticeAmount;
// 	//materialAmount += obj.materialAmount;
// 	////++objectAmount;
//
// 	////resizing	
// 	//vertices.Resize(vertexAmount);
// 	//faces.Resize(faceAmount);
// 	//mortonCodes.ResizeDevice(faceAmount);
// 	//boxes.Resize(faceAmount);
// 	//tets.Resize(tetAmount);
// 	//materials.Resize(materialAmount);
// 	////objects.Resize(objectAmount);
// 	//
//
// 	////update the scene's bounding volume
// 	//sceneBox.max = glm::max(sceneBox.max, obj.box.max);
// 	//sceneBox.min = glm::min(sceneBox.min, obj.box.min);
//
// 	////create the minified object from the input object
// 	////objects[objectOffset] = MiniObject(obj);
//
// 	//const auto maxIter =
// 	//	glm::max(obj.materialAmount,
// 	//		glm::max(obj.verticeAmount,
// 	//			glm::max(obj.faceAmount,
// 	//				obj.tetAmount
// 	//			)));
//
// 	//for (uint t = 0; t < maxIter; ++t) {
// 	//	if (t < obj.verticeAmount) {
// 	//		vertices[t + vertexOffset] = obj.vertices[t];
// 	//		//vertices[t + vertexOffset].object = objectOffset;
// 	//	}
// 	//	if (t < obj.faceAmount) {
// 	//		faces[t + faceOffset] = obj.faces[t];
//
// 	//		glm::uvec3& ind = faces[t + faceOffset].indices;
//
// 	//		//before adding the offsets, updated the bounding box
// 	//		
// 	//		const glm::vec3 pos0 = obj.vertices[ind.x].position;
// 	//		const glm::vec3 pos1 = obj.vertices[ind.y].position;
// 	//		const glm::vec3 pos2 = obj.vertices[ind.z].position;
//
// 	//		glm::vec3 max = pos0;
// 	//		glm::vec3 min = pos0;
//
// 	//		max = glm::max(pos1, max);
// 	//		min = glm::min(pos1, min);
//
// 	//		max = glm::max(pos2, max);
// 	//		min = glm::min(pos2, min);
//
// 	//		BoundingBox& box = boxes[t + faceOffset];
// 	//		box.max = max;
// 	//		box.min = min;
//
// 	//		ind.x += vertexOffset;
// 	//		ind.y += vertexOffset;
// 	//		ind.z += vertexOffset;
//
// 	//		faces[t + faceOffset].material += materialOffset;			
// 	//	}
// 	//	if (t < obj.tetAmount) {
// 	//		tets[t + tetOffset] = obj.tets[t];
// 	//		tets[t + tetOffset].material += materialOffset;
// 	//		//tets[t + tetOffset].object = objectOffset;
// 	//	}
// 	//	if (t < obj.materialAmount) {
// 	//		//materials[t + objectOffset] = obj.materials[t];
// 	//	}
// 	//}
//
// }
//
// void Scene::RemoveObject(Object& obj) {
//
// 	//S_LOG_WARNING("Removal of objects from a scene not yet implemented");
// }
