export module synodic.soul.render.graph:builder;

import synodic.soul.core;
import synodic.soul.raster;
import :parameter;
import :pass;
import :resource;
import std;

// Forward declaration
class RenderGraph;

// Fluent builder for render passes
export class RenderPassBuilder final {
public:
	explicit RenderPassBuilder(std::string name) {
		desc_.name = std::move(name);
		desc_.type = PassType::Raster;
	}

	~RenderPassBuilder() = default;

	RenderPassBuilder(const RenderPassBuilder&) = delete;
	RenderPassBuilder(RenderPassBuilder&&) noexcept = default;

	RenderPassBuilder& operator=(const RenderPassBuilder&) = delete;
	RenderPassBuilder& operator=(RenderPassBuilder&&) noexcept = default;

	// Set shaders
	RenderPassBuilder& Shaders(const ShaderSet& shaders) {
		desc_.shaders = shaders;
		return *this;
	}

	// Add color output attachment
	RenderPassBuilder& ColorOutput(ResourceHandle handle, ResourceUsage additionalUsage = ResourceUsage::None) {
		ResourceRef ref;
		ref.handle = handle;
		ref.usage = ResourceUsage::ColorAttachmentWrite | additionalUsage;
		desc_.colorOutputs.push_back(ref);
		return *this;
	}

	// Add depth/stencil output
	RenderPassBuilder& DepthStencilOutput(ResourceHandle handle, bool readOnly = false) {
		ResourceRef ref;
		ref.handle = handle;
		ref.usage = readOnly
			? ResourceUsage::DepthStencilRead
			: (ResourceUsage::DepthStencilRead | ResourceUsage::DepthStencilWrite);
		desc_.depthStencilOutput = ref;
		return *this;
	}

	// Add depth-only output (no stencil, write-only)
	RenderPassBuilder& DepthOutput(ResourceHandle handle, ResourceUsage additionalUsage = ResourceUsage::None) {
		ResourceRef ref;
		ref.handle = handle;
		ref.usage = ResourceUsage::DepthStencilWrite | additionalUsage;
		desc_.depthStencilOutput = ref;
		return *this;
	}

	// Add depth input (read-only depth testing)
	RenderPassBuilder& DepthInput(ResourceHandle handle, ResourceUsage additionalUsage = ResourceUsage::None) {
		ResourceRef ref;
		ref.handle = handle;
		ref.usage = ResourceUsage::DepthStencilRead | additionalUsage;
		desc_.depthStencilOutput = ref;
		return *this;
	}

	// Add input resource (sampled texture, uniform buffer, etc.)
	RenderPassBuilder& Input(ResourceHandle handle, ResourceUsage usage) {
		ResourceRef ref;
		ref.handle = handle;
		ref.usage = usage;
		desc_.inputs.push_back(ref);
		return *this;
	}

	// Shorthand for fragment shader sampled input
	RenderPassBuilder& SampledInput(ResourceHandle handle) {
		return Input(handle, ResourceUsage::FragmentShaderRead);
	}

	// Shorthand for vertex shader input
	RenderPassBuilder& VertexInput(ResourceHandle handle) {
		return Input(handle, ResourceUsage::VertexShaderRead);
	}

	// Set clear color
	RenderPassBuilder& ClearColor(float r, float g, float b, float a = 1.0f) {
		desc_.clearColor = {r, g, b, a};
		desc_.shouldClear = true;
		return *this;
	}

	// Set clear depth/stencil
	RenderPassBuilder& ClearDepth(float depth, std::uint32_t stencil = 0) {
		desc_.clearDepth = depth;
		desc_.clearStencil = stencil;
		return *this;
	}

	// Don't clear - load previous contents
	RenderPassBuilder& LoadPrevious() {
		desc_.shouldClear = false;
		desc_.loadPreviousContent = true;
		return *this;
	}

	// Don't store result (transient only)
	RenderPassBuilder& DontStore() {
		desc_.storeResult = false;
		return *this;
	}

	// Build and return the descriptor
	[[nodiscard]] RenderPassDesc Build() {
		return std::move(desc_);
	}

private:
	RenderPassDesc desc_;
};

// Fluent builder for compute passes
export class ComputePassBuilder final {
public:
	explicit ComputePassBuilder(std::string name) {
		desc_.name = std::move(name);
	}

	~ComputePassBuilder() = default;

	ComputePassBuilder(const ComputePassBuilder&) = delete;
	ComputePassBuilder(ComputePassBuilder&&) noexcept = default;

	ComputePassBuilder& operator=(const ComputePassBuilder&) = delete;
	ComputePassBuilder& operator=(ComputePassBuilder&&) noexcept = default;

	// Set shaders
	ComputePassBuilder& Shaders(const ShaderSet& shaders) {
		desc_.shaders = shaders;
		return *this;
	}

	// Add input resource
	ComputePassBuilder& Input(ResourceHandle handle, ResourceUsage usage = ResourceUsage::ComputeShaderRead) {
		ResourceRef ref;
		ref.handle = handle;
		ref.usage = usage;
		desc_.inputs.push_back(ref);
		return *this;
	}

	// Add output resource
	ComputePassBuilder& Output(ResourceHandle handle, ResourceUsage usage = ResourceUsage::ComputeShaderWrite) {
		ResourceRef ref;
		ref.handle = handle;
		ref.usage = usage;
		desc_.outputs.push_back(ref);
		return *this;
	}

	// Set dispatch size
	ComputePassBuilder& Dispatch(std::uint32_t x, std::uint32_t y = 1) {
		desc_.dispatchSize = {x, y};
		return *this;
	}

	// Build and return the descriptor
	[[nodiscard]] ComputePassDesc Build() {
		return std::move(desc_);
	}

private:
	ComputePassDesc desc_;
};

// Legacy builder for compatibility with existing code
export class RenderGraphBuilder final {
public:
	RenderGraphBuilder(std::shared_ptr<RasterModule>&, std::shared_ptr<EntityRegistry>&, Entity, bool);
	~RenderGraphBuilder();

	RenderGraphBuilder(const RenderGraphBuilder&) = delete;
	RenderGraphBuilder(RenderGraphBuilder&&) noexcept = default;

	RenderGraphBuilder& operator=(const RenderGraphBuilder&) = delete;
	RenderGraphBuilder& operator=(RenderGraphBuilder&&) noexcept = default;

	void CreateOutput(RenderGraphOutputParameters&);
	void CreateInput(RenderGraphInputParameters&);

	void CreateSubpass();

	template<class T>
	Entity Request();

	Entity View();

private:
	std::shared_ptr<EntityRegistry> entityRegistry_;
	std::shared_ptr<RasterModule> rasterModule_;

	Entity renderPass_;
	bool subPass_;
};

template<class T>
Entity RenderGraphBuilder::Request() {
	// TODO: C++20 Concepts
	static_assert(std::is_base_of<RenderResource, T>::value,
		"The type parameter must be a subclass of RenderResource");

	Entity returnedEntity = entityRegistry_->CreateEntity();
	entityRegistry_->AttachComponent<T>(returnedEntity);

	return returnedEntity;
}
