#include "VulkanShader.h"
#include "Device/VulkanDevice.h"
#include "Core/Utility/Exception/Exception.h"
#include "Transput/Resource/Resource.h"

#include <fstream>
#include <filesystem>

VulkanShader::VulkanShader(const vk::Device& device,
	const vk::ShaderStageFlagBits& shaderType,
	const Resource& resource):
	device_(device)
{

	const auto& path = resource.Path();
	
	if (!std::filesystem::exists(path)) {
		throw NotImplemented();
	}

	// TODO: abstract into some sort of loader
	std::ifstream file(path.c_str(), std::ifstream::ate | std::ios::binary);

	const size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<std::byte> buffer(fileSize);

	file.seekg(0);
	file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
	file.close();

	vk::ShaderModuleCreateInfo createInfo;
	createInfo.codeSize = buffer.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

	module_ = device.createShaderModule(createInfo, nullptr);

    info_.stage = shaderType;
	info_.module = module_;
	info_.pName = "main";

}

VulkanShader::~VulkanShader() {

	device_.destroyShaderModule(module_, nullptr);

}

const vk::PipelineShaderStageCreateInfo& VulkanShader::PipelineInfo() const
{

	return info_;

}
