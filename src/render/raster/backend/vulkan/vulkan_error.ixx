export module synodic.soul.raster.backend.vulkan:error;

import std;
import vulkan;

// Vulkan-specific error codes for std::expected
export enum class VulkanError {
	Success = 0,

	// Memory errors
	AllocationFailed,
	MemoryTypeNotFound,
	OutOfDeviceMemory,
	OutOfHostMemory,
	ReBarNotSupported,  // Resizable BAR not available for CPU-mapped GPU memory

	// Device errors
	DeviceNotFound,
	DeviceLost,
	SurfaceNotSupported,
	SurfaceLost,
	FeatureNotSupported,  // Required Vulkan feature not supported

	// Swapchain errors
	SwapchainOutOfDate,
	SwapchainAcquireFailed,
	NoPresentModesAvailable,

	// Pipeline errors
	PipelineCreationFailed,
	ShaderCompilationFailed,

	// Bindless errors
	TextureHeapFull,
	InvalidTextureIndex,
	DescriptorBufferNotSupported,

	// Synchronization errors
	FenceTimeout,

	// General errors
	NotImplemented,
	InvalidArgument,
	InitializationFailed,
	UnknownError
};

// Convert Vulkan result to VulkanError
export constexpr VulkanError FromVkResult(vk::Result result) noexcept {
	switch (result) {
		case vk::Result::eSuccess:
			return VulkanError::Success;
		case vk::Result::eErrorOutOfHostMemory:
			return VulkanError::OutOfHostMemory;
		case vk::Result::eErrorOutOfDeviceMemory:
			return VulkanError::OutOfDeviceMemory;
		case vk::Result::eErrorDeviceLost:
			return VulkanError::DeviceLost;
		case vk::Result::eErrorOutOfDateKHR:
			return VulkanError::SwapchainOutOfDate;
		case vk::Result::eErrorSurfaceLostKHR:
			return VulkanError::SurfaceLost;
		default:
			return VulkanError::UnknownError;
	}
}

// Convert VulkanError to string for debugging
export constexpr const char* ToString(VulkanError error) noexcept {
	switch (error) {
		case VulkanError::Success:
			return "Success";
		case VulkanError::AllocationFailed:
			return "Allocation failed";
		case VulkanError::MemoryTypeNotFound:
			return "Memory type not found";
		case VulkanError::OutOfDeviceMemory:
			return "Out of device memory";
		case VulkanError::OutOfHostMemory:
			return "Out of host memory";
		case VulkanError::ReBarNotSupported:
			return "Resizable BAR not available for CPU-mapped GPU memory";
		case VulkanError::DeviceNotFound:
			return "Device not found";
		case VulkanError::DeviceLost:
			return "Device lost";
		case VulkanError::SurfaceNotSupported:
			return "Surface not supported by device";
		case VulkanError::SurfaceLost:
			return "Surface lost";
		case VulkanError::FeatureNotSupported:
			return "Required Vulkan feature not supported";
		case VulkanError::SwapchainOutOfDate:
			return "Swapchain out of date";
		case VulkanError::SwapchainAcquireFailed:
			return "Failed to acquire swapchain image";
		case VulkanError::NoPresentModesAvailable:
			return "No present modes available";
		case VulkanError::PipelineCreationFailed:
			return "Pipeline creation failed";
		case VulkanError::ShaderCompilationFailed:
			return "Shader compilation failed";
		case VulkanError::TextureHeapFull:
			return "Texture heap is full";
		case VulkanError::InvalidTextureIndex:
			return "Invalid texture index";
		case VulkanError::DescriptorBufferNotSupported:
			return "VK_EXT_descriptor_buffer not supported";
		case VulkanError::FenceTimeout:
			return "Fence wait timeout";
		case VulkanError::NotImplemented:
			return "Functionality not yet implemented";
		case VulkanError::InvalidArgument:
			return "Invalid argument";
		case VulkanError::InitializationFailed:
			return "Initialization failed";
		case VulkanError::UnknownError:
			return "Unknown error";
		default:
			return "Undefined error";
	}
}

// Helper type alias for common expected pattern
export template<typename T>
using VulkanResult = std::expected<T, VulkanError>;

// Void result for operations that don't return a value
export using VulkanVoidResult = std::expected<void, VulkanError>;
