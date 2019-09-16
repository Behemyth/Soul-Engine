//#include "Image.cuh"
//
//#pragma warning(push, 0)        
//#define STB_IMAGE_IMPLEMENTATION  
//#include "stb_image.h"
//#pragma warning(pop)
//
//
//#include "Types.h"
//#include "Parallelism/Compute/CUDA/Utility/CUDAMath.h"
//#include "Parallelism/Compute/CUDA/Utility/CUDAHelper.cuh"
//#include <device_launch_parameters.h>
//#include <vector_types.h>
//
//#define MAX(a,b) ((a > b) ? a : b)
//
//
//Image::Image()
//	:width(0)
//	, height(0)
//	, format(0)
//{
//
//}
////
////Image::~Image() {
////
////	CudaCheck(cudaDestroyTextureObject(texObj));
////
////}
//
//
//#pragma pack(push,4)
//struct cuImage
//{
//	void                   *h_data;
//	cudaExtent              size;
//	cudaResourceType        type;
//	cudaArray_t             dataArray;
//	cudaMipmappedArray_t    mipmapArray;
//	cudaTextureObject_t     textureObject;
//
//	cuImage()
//	{
//		memset(this, 0, sizeof(Image));
//	}
//};
//
//uint getMipMapLevels(cudaExtent size)
//{
//	size_t sz = MAX(MAX(size.width, size.height), size.depth);
//
//	uint levels = 0;
//
//	while (sz)
//	{
//		sz /= 2;
//		levels++;
//	}
//
//	return levels;
//}
//inline void _checkHost(bool test, const char *condition, const char *file, int line, const char *func)
//{
//	if (!test)
//	{
//		fprintf(stderr, "HOST error at %s:%d (%s) /"%s/" /n",
//			file, line, condition, func);
//		exit(EXIT_FAILURE);
//	}
//}
//
//#define checkHost(condition)   _checkHost(condition, #condition,__FILE__,__LINE__,__FUNCTION__)
//
//
//
//
//
//
//__device__ __inline__ uchar4 to_uchar4(float4 vec)
//{
//	return make_uchar4((unsigned char)vec.x, (unsigned char)vec.y, (unsigned char)vec.z, (unsigned char)vec.w);
//}
//
//
//
//
//__global__ void
//d_mipmap(cudaSurfaceObject_t mipOutput, cudaTextureObject_t mipInput, uint imageW, uint imageH)
//{
//	uint x = blockIdx.x * blockDim.x + threadIdx.x;
//	uint y = blockIdx.y * blockDim.y + threadIdx.y;
//
//	float px = 1.0 / float(imageW);
//	float py = 1.0 / float(imageH);
//
//
//	if ((x < imageW) && (y < imageH))
//	{
//		// take the average of 4 samples
//
//		// we are using the normalized access to make sure non-power-of-two textures
//		// behave well when downsized.
//		float4 color =
//			(tex2D<float4>(mipInput, (x + 0) * px, (y + 0) * py)) +
//			(tex2D<float4>(mipInput, (x + 1) * px, (y + 0) * py)) +
//			(tex2D<float4>(mipInput, (x + 1) * px, (y + 1) * py)) +
//			(tex2D<float4>(mipInput, (x + 0) * px, (y + 1) * py));
//
//
//		color.x /= 4.0;
//		color.y /= 4.0;
//		color.z /= 4.0;
//		color.w /= 4.0;
//		color.x *= 255.0;
//		color.y *= 255.0;
//		color.z *= 255.0;
//		color.w *= 255.0;
//		color = fminf(color, make_float4(255.0, 255.0, 255.0, 255.0));
//
//		surf2Dwrite(to_uchar4(color), mipOutput, x * sizeof(uchar4), y);
//	}
//}
//
//
//
//void generateMipMaps(cudaMipmappedArray_t mipmapArray, cudaExtent size)
//{
//	size_t width = size.width;
//	size_t height = size.height;
//
//
//	uint level = 0;
//
//	while (width != 1 || height != 1)
//	{
//		width /= 2;
//		width = MAX((size_t)1, width);
//		height /= 2;
//		height = MAX((size_t)1, height);
//
//		cudaArray_t levelFrom;
//		CudaCheck(cudaGetMipmappedArrayLevel(&levelFrom, mipmapArray, level));
//		cudaArray_t levelTo;
//		CudaCheck(cudaGetMipmappedArrayLevel(&levelTo, mipmapArray, level + 1));
//
//		cudaExtent  levelToSize;
//		CudaCheck(cudaArrayGetInfo(nullptr, &levelToSize, nullptr, levelTo));
//		checkHost(levelToSize.width == width);
//		checkHost(levelToSize.height == height);
//		checkHost(levelToSize.depth == 0);
//
//		// generate texture object for reading
//		cudaTextureObject_t         texInput;
//		cudaResourceDesc            texRes;
//		memset(&texRes, 0, sizeof(cudaResourceDesc));
//
//		texRes.resType = cudaResourceTypeArray;
//		texRes.res.array.array = levelFrom;
//
//		cudaTextureDesc             texDescr;
//		memset(&texDescr, 0, sizeof(cudaTextureDesc));
//
//		texDescr.normalizedCoords = 1;
//		texDescr.filterMode = cudaFilterModeLinear;
//
//		texDescr.addressMode[0] = cudaAddressModeClamp;
//		texDescr.addressMode[1] = cudaAddressModeClamp;
//		texDescr.addressMode[2] = cudaAddressModeClamp;
//
//		texDescr.readMode = cudaReadModeNormalizedFloat;
//
//		CudaCheck(cudaCreateTextureObject(&texInput, &texRes, &texDescr, nullptr));
//
//		// generate surface object for writing
//
//		cudaSurfaceObject_t surfOutput;
//		cudaResourceDesc    surfRes;
//		memset(&surfRes, 0, sizeof(cudaResourceDesc));
//		surfRes.resType = cudaResourceTypeArray;
//		surfRes.res.array.array = levelTo;
//
//		CudaCheck(cudaCreateSurfaceObject(&surfOutput, &surfRes));
//
//		// run mipmap kernel
//		dim3 blockSize(16, 16, 1);
//		dim3 gridSize(((uint)width + blockSize.x - 1) / blockSize.x, ((uint)height + blockSize.y - 1) / blockSize.y, 1);
//
//		d_mipmap << <gridSize, blockSize >> >(surfOutput, texInput, (uint)width, (uint)height);
//
//		CudaCheck(cudaPeekAtLastError());
//		CudaCheck(cudaDeviceSynchronize());
//		CudaCheck(cudaGetLastError());
//
//		CudaCheck(cudaDestroySurfaceObject(surfOutput));
//
//		CudaCheck(cudaDestroyTextureObject(texInput));
//
//		level++;
//	}
//}
//
////rgba
//unsigned char* DirectLoad(const char* fileName, int* width, int* height, int*texChannels){
//	return stbi_load(fileName, width, height, texChannels, STBI_rgb_alpha);
//}
//
//
//void Image::LoadFromFile(const char* filepath, bool clamp, bool mipmap) {
//	void* pixels = stbi_load(filepath, &width, &height, &format, 0);
//
//	cuImage im;
//	im.size = make_cudaExtent(width, height, 0);
//	im.size.depth = 0;
//	im.type = cudaResourceTypeMipmappedArray;
//
//	uint levels;
//	if (mipmap){
//		levels = getMipMapLevels(im.size);
//	}
//	else{
//		levels = 1;
//	}
//
//	cudaChannelFormatDesc desc = cudaCreateChannelDesc<uchar4>();
//	CudaCheck(cudaMallocMipmappedArray(&im.mipmapArray, &desc, im.size, levels));
//
//	cudaArray_t level0;
//	CudaCheck(cudaGetMipmappedArrayLevel(&level0, im.mipmapArray, 0));
//
//	cudaMemcpy3DParms copyParams = { 0 };
//	copyParams.srcPtr = make_cudaPitchedPtr(pixels, im.size.width * sizeof(uchar4), im.size.width, im.size.height);
//	copyParams.dstArray = level0;
//	copyParams.extent = im.size;
//	copyParams.extent.depth = 1;
//	copyParams.kind = cudaMemcpyHostToDevice;
//	CudaCheck(cudaMemcpy3D(&copyParams));
//
//	// compute rest of mipmaps based on level 0
//	if (mipmap){
//		generateMipMaps(im.mipmapArray, im.size);
//	}
//
//	// generate bindless texture object
//
//	cudaResourceDesc            resDescr;
//	memset(&resDescr, 0, sizeof(cudaResourceDesc));
//
//	resDescr.resType = cudaResourceTypeMipmappedArray;
//	resDescr.res.mipmap.mipmap = im.mipmapArray;
//
//	cudaTextureDesc             texDescr;
//	memset(&texDescr, 0, sizeof(cudaTextureDesc));
//
//	texDescr.normalizedCoords = true;
//	texDescr.filterMode = cudaFilterModeLinear;
//	texDescr.mipmapFilterMode = cudaFilterModeLinear;
//
//	if (!clamp){
//		texDescr.addressMode[0] = cudaAddressModeWrap;
//		texDescr.addressMode[1] = cudaAddressModeWrap;
//		texDescr.addressMode[2] = cudaAddressModeWrap;
//	}
//	else{
//		texDescr.addressMode[0] = cudaAddressModeClamp;
//		texDescr.addressMode[1] = cudaAddressModeClamp;
//		texDescr.addressMode[2] = cudaAddressModeClamp;
//	}
//
//	texDescr.maxMipmapLevelClamp = float(levels - 1);
//
//	texDescr.readMode = cudaReadModeNormalizedFloat;
//
//	CudaCheck(cudaCreateTextureObject(&texObj, &resDescr, &texDescr, nullptr));
//
//	if (pixels) {
//		delete[](char*)pixels;
//	}
//
//}