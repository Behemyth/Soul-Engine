export module synodic.soul.raster.backend.mock;

import synodic.soul.raster;

export class MockRasterBackend : public RasterModule
{
public:
	MockRasterBackend()			  = default;
	~MockRasterBackend() override = default;

	MockRasterBackend(const MockRasterBackend&)		= delete;
	MockRasterBackend(MockRasterBackend&&) noexcept = default;

	MockRasterBackend& operator=(const MockRasterBackend&)	   = delete;
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

	void CreatePassInput(Entity, Entity, Format) override
	{
	}

	void CreatePassOutput(Entity, Entity, Format) override
	{
	}

	Entity CreateSurface(NativeSurfaceHandle, uvec2) override
	{
		return Entity();
	}

	void UpdateSurface(Entity, uvec2) override
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

	// Agnostic raster API interface
	void Compile(CommandList&) override
	{
	}
};
