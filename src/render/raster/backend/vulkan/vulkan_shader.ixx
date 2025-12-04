
export module synodic.soul.raster.backend.vulkan:shader;
import std;
import vulkan_hpp;
import synodic.soul.transput;

export class VulkanShader {

public:

	// Load shader from a pre-compiled SPIR-V file (Resource)
	VulkanShader(const vk::Device&, const vk::ShaderStageFlagBits&, const Resource&);

	// Load shader from a pre-compiled SPIR-V file path
	VulkanShader(const vk::Device&, vk::ShaderStageFlagBits stage, const std::filesystem::path& spvPath,
		std::string_view entryPoint = "main");

	// Load shader from SPIR-V byte data
	VulkanShader(const vk::Device&, vk::ShaderStageFlagBits stage, std::span<const std::byte> spirvCode,
		std::string_view entryPoint = "main");

	~VulkanShader();

	VulkanShader(const VulkanShader&) = delete;
	VulkanShader(VulkanShader&& o) noexcept;

	VulkanShader& operator=(const VulkanShader&) = delete;
	VulkanShader& operator=(VulkanShader&& other) noexcept;

	[[nodiscard]] const vk::PipelineShaderStageCreateInfo& PipelineInfo() const;


private:

	void CreateShaderModule(std::span<const std::byte> spirvCode);

	vk::ShaderModule module_;
	vk::PipelineShaderStageCreateInfo info_;
	std::string entryPointName_;  // Storage for entry point name

	vk::Device device_;

};
