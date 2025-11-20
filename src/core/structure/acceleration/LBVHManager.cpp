module synodic.soul.engine;
//#include "CUDA/BVH.cuh"
//#include "Parallelism/ComputeOld/ComputeManager.h"

//LBVHManager::LBVHManager() :
//	nodes(S_BEST_DEVICE)
//{
//
//}
//
//void LBVHManager::Build(std::int32_t size, ComputeBuffer<BVH>& data, ComputeBuffer<std::int64_t>& mortonCodes, ComputeBuffer<BoundingBox>& boxes) {
//
//	if (size > 0) {
//
//		const GPUExecutePolicy normalPolicy(size, 64, 0, 0);
//
//		nodes.ResizeDevice(size); // size-1 nodes are actually used. 1 is tacked on to remove conditional statements
//
//		data[0].leafSize = size;
//		data[0].nodes = nodes.DataDevice();
//		data[0].boxes = boxes.DataDevice();
//		data.TransferToDevice();
//
//		ComputeDevice device = S_BEST_DEVICE;
//
//
//		device.Launch(normalPolicy, Reset,
//			size,
//			nodes.DataDevice());
//
//		device.Launch(normalPolicy, BuildTree,
//			size,
//			data.DataDevice(),
//			nodes.DataDevice(),
//			mortonCodes.DataDevice(),
//			boxes.DataDevice());
//
//	}
//}
