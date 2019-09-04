#pragma once

#include "Core/Interface/Module/Module.h"

#include "Types.h"
#include "RenderCommands.h"
#include "RenderResource.h"
#include "RenderTypes.h"

#include <memory>
#include <any>
#include <functional>

class Scheduler;
class Window;
class RasterDevice;
class WindowParameters;
class SchedulerModule;
class WindowModule;
class CommandList;

class RasterModule : public Module<RasterModule> {

public:

	RasterModule();
	virtual ~RasterModule() = default;

	RasterModule(const RasterModule &) = delete;
	RasterModule(RasterModule &&) noexcept = default;

	RasterModule& operator=(const RasterModule &) = delete;
	RasterModule& operator=(RasterModule &&) noexcept = default;

	virtual void Present() = 0;

	virtual Entity CreatePass(const ShaderSet&, std::function<void(Entity)>) = 0;
	virtual Entity CreateSubPass(Entity, const ShaderSet&, std::function<void(Entity)>) = 0;
	virtual void ExecutePass(Entity, Entity, CommandList&) = 0;


	virtual void CreatePassInput(Entity, Entity, Format) = 0;
	virtual void CreatePassOutput(Entity, Entity, Format) = 0;

	virtual Entity CreateSurface(std::any, glm::uvec2) = 0;
	virtual void UpdateSurface(Entity, glm::uvec2) = 0;
	virtual void RemoveSurface(Entity) = 0;
	virtual void AttachSurface(Entity, Entity) = 0;
	virtual void DetachSurface(Entity, Entity) = 0;

	// Agnostic raster API interface
	virtual void Compile(CommandList&) = 0;

	//Factory
	static std::shared_ptr<RasterModule> CreateModule(std::shared_ptr<SchedulerModule>&,
		std::shared_ptr<EntityRegistry>&,
		std::shared_ptr<WindowModule>&);

};
