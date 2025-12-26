export module synodic.soul.raster.backend.mock;

import std;
import synodic.periapsis;
import synodic.soul.raster;

export class MockRasterBackend : public RasterModule
{
public:
	MockRasterBackend()           = default;
	~MockRasterBackend() override = default;

	MockRasterBackend(const MockRasterBackend&)     = delete;
	MockRasterBackend(MockRasterBackend&&) noexcept = default;

	MockRasterBackend& operator=(const MockRasterBackend&)     = delete;
	MockRasterBackend& operator=(MockRasterBackend&&) noexcept = default;

	void Present() override
	{
	}

	Entity CreatePass(const ShaderSet&, std::function<void(Entity)>) override
	{
		return Entity();
	}

	Entity CreateSubPass(Entity, const ShaderSet&, std::function<void(Entity)>) override
	{
		return Entity();
	}

	void ExecutePass(Entity, Entity, CommandList&) override
	{
	}

	void ExecutePassWithFlags(Entity, Entity, CommandList&, PassExecutionFlags) override
	{
	}

	void CreatePassInput(Entity, Entity, Format) override
	{
	}

	void CreatePassOutput(Entity, Entity, Format) override
	{
	}

	Entity CreateSurface(NativeSurfaceHandle, peri::math::uvec2) override
	{
		return Entity();
	}

	void UpdateSurface(Entity, peri::math::uvec2) override
	{
	}

	void RemoveSurface(Entity) override
	{
	}

	void AttachSurface(Entity, Entity) override
	{
	}

	void DetachSurface(Entity, Entity) override
	{
	}

	// Buffer management
	GPUBufferHandle CreateBuffer(const BufferDesc&) override
	{
		return nextBufferHandle_++;
	}

	void DestroyBuffer(GPUBufferHandle) override
	{
	}

	GPUDeviceAddress GetBufferGPUAddress(GPUBufferHandle) override
	{
		return InvalidGPUAddress;
	}

	void UploadBufferData(GPUBufferHandle, const void*, std::size_t, std::size_t) override
	{
	}

	void* MapBuffer(GPUBufferHandle) override
	{
		return nullptr;
	}

	void UnmapBuffer(GPUBufferHandle) override
	{
	}

	void FlushBuffer(GPUBufferHandle, std::size_t, std::size_t) override
	{
	}

	FrameAllocator* GetFrameAllocator() override
	{
		return nullptr;
	}

	void ResetFrameAllocator() override
	{
	}

	// Agnostic raster API interface
	void Compile(CommandList&) override
	{
	}

private:
	GPUBufferHandle nextBufferHandle_ = 1;
};

