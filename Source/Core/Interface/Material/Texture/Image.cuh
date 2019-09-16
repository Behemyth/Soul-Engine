//#pragma once
//
//#include <cuda_runtime.h>
//
//class Image{
//public:
//
//	Image();
//	void LoadFromFile(const char*, bool, bool);
//	//~Image();
//
//	int width;
//	int height;
//	int format;
//
//	cudaTextureObject_t texObj;
//
//	__host__ __device__ bool operator==(const Image& other) const {
//		return
//			width == other.width &&
//			height == other.height &&
//			format == other.format &&
//			texObj == other.texObj;
//	}
//
//	__host__ __device__ friend void swap(Image& a, Image& b)
//	{
//
//		int temp = a.width;
//		a.width = b.width;
//		b.width = temp;
//
//		temp = a.height;
//		a.height = b.height;
//		b.height = temp;
//
//		temp = a.format;
//		a.format = b.format;
//		b.format = temp;
//
//		cudaTextureObject_t temp1 = a.texObj;
//		a.texObj = b.texObj;
//		b.texObj = temp1;
//
//	}
//	__host__ __device__ Image& operator=(Image arg)
//	{
//		this->width = arg.width;
//		this->height = arg.height;
//		this->format = arg.format;
//		this->texObj = arg.texObj;
//
//		return *this;
//	}
//private:
//
//};
//
//unsigned char* DirectLoad(const char*, int*, int*, int*);