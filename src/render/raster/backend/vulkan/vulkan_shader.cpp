module synodic.soul.raster.backend.vulkan;

import synodic.soul.core;
import synodic.soul.transput;

VulkanShader::VulkanShader(const vk::Device& device,
	const vk::ShaderStageFlagBits& shaderType,
	const Resource& resource):
	device_(device),
	entryPointName_("main")
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

	CreateShaderModule(buffer);

	info_.stage = shaderType;
	info_.module = module_;
	info_.pName = entryPointName_.c_str();
}

VulkanShader::VulkanShader(const vk::Device& device,
	vk::ShaderStageFlagBits stage,
	const std::filesystem::path& spvPath,
	std::string_view entryPoint):
	device_(device),
	entryPointName_(entryPoint)
{
	if (!std::filesystem::exists(spvPath)) {
		throw NotImplemented();
	}

	std::ifstream file(spvPath, std::ios::ate | std::ios::binary);
	const size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<std::byte> buffer(fileSize);

	file.seekg(0);
	file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
	file.close();

	CreateShaderModule(buffer);

	info_.stage = stage;
	info_.module = module_;
	info_.pName = entryPointName_.c_str();
}

VulkanShader::VulkanShader(const vk::Device& device,
	vk::ShaderStageFlagBits stage,
	std::span<const std::byte> spirvCode,
	std::string_view entryPoint):
	device_(device),
	entryPointName_(entryPoint)
{
	CreateShaderModule(spirvCode);

	info_.stage = stage;
	info_.module = module_;
	info_.pName = entryPointName_.c_str();
}

void VulkanShader::CreateShaderModule(std::span<const std::byte> spirvCode)
{
	vk::ShaderModuleCreateInfo createInfo;
	createInfo.codeSize = spirvCode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(spirvCode.data());

	module_ = device_.createShaderModule(createInfo, nullptr);
}

VulkanShader::~VulkanShader() {
	if (module_ && device_) {
		device_.destroyShaderModule(module_, nullptr);
	}
}

VulkanShader::VulkanShader(VulkanShader&& o) noexcept :
	module_(o.module_),
	info_(o.info_),
	entryPointName_(std::move(o.entryPointName_)),
	device_(o.device_)
{
	o.module_ = nullptr;
	o.device_ = nullptr;
	// Update pName to point to our own string storage
	info_.pName = entryPointName_.c_str();
}

VulkanShader& VulkanShader::operator=(VulkanShader&& other) noexcept {
	if (this != &other) {
		if (module_ && device_) {
			device_.destroyShaderModule(module_, nullptr);
		}
		module_ = other.module_;
		info_ = other.info_;
		entryPointName_ = std::move(other.entryPointName_);
		device_ = other.device_;
		other.module_ = nullptr;
		other.device_ = nullptr;
		// Update pName to point to our own string storage
		info_.pName = entryPointName_.c_str();
	}
	return *this;
}

const vk::PipelineShaderStageCreateInfo& VulkanShader::PipelineInfo() const
{
	return info_;
}
