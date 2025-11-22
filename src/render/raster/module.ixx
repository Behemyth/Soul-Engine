export module synodic.soul.raster;

import std;

import synodic.soul.core;

export import :types;
export import :resource;
export import :commands;
export import :device;
export import :command_list;

// Note: Forward declarations used here to avoid circular dependencies
// These types are only used in shared_ptr parameters in factory methods
export class SchedulerBackend;
export class EntityRegistry;
export class WindowModule;

export class RasterModule
{
public:
	RasterModule() = default;
	virtual ~RasterModule() = default;

	RasterModule(const RasterModule&)	  = delete;
	RasterModule(RasterModule&&) noexcept = default;

	RasterModule& operator=(const RasterModule&)	 = delete;
	RasterModule& operator=(RasterModule&&) noexcept = default;

	virtual void Present() = 0;

	virtual Entity CreatePass(const ShaderSet&, std::function<void(Entity)>)			= 0;
	virtual Entity CreateSubPass(Entity, const ShaderSet&, std::function<void(Entity)>) = 0;
	virtual void ExecutePass(Entity, Entity, CommandList&)								= 0;

	virtual void CreatePassInput(Entity, Entity, Format)  = 0;
	virtual void CreatePassOutput(Entity, Entity, Format) = 0;

	virtual Entity CreateSurface(std::any, uvec2) = 0;
	virtual void UpdateSurface(Entity, uvec2)	  = 0;
	virtual void RemoveSurface(Entity)			  = 0;
	virtual void AttachSurface(Entity, Entity)	  = 0;
	virtual void DetachSurface(Entity, Entity)	  = 0;

	// Agnostic raster API interface
	virtual void Compile(CommandList&) = 0;
};
