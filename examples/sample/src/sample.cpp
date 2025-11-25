import std;
import synodic.soul.core;
import synodic.soul.engine;
import synodic.soul.scheduler;

import synodic.soul.raster.backend.vulkan;
import synodic.soul.window.backend.sdl;
import synodic.soul.input.backend.sdl;
import synodic.soul.raster.backend.mock;
import synodic.soul.gui.backend.standard;
import synodic.soul.render.graph.backend.standard;
import synodic.soul.compute.backend.mock;
import synodic.soul.scheduler.backend.passthrough;
import synodic.soul.backend.sdl;

using SampleApp = synodic::soul::App<
	PassthroughSchedulerBackend,
	MockBackend,
	SDLInputBackend,
	VulkanRasterBackend<PassthroughSchedulerBackend>,
	StandardRenderGraphBackend,
	SDLWindowBackend,
	StandardGUIBackend>;

class Sample : public SampleApp
{
public:
	explicit Sample(
		const synodic::soul::Parameters& params,
		SDLInputBackend inputBackend,
		SDLWindowBackend windowBackend,
		PassthroughSchedulerBackend& schedulerBackend) :
		SampleApp(
			params,
			std::move(inputBackend),
			std::move(windowBackend),
			VulkanRasterBackend(schedulerBackend))
	{
	}

protected:
	void OnInit() override
	{
		if (GetSoul().Window().has_value())
		{
			WindowParameters winParams;
			winParams.title			= "Sample Application";
			winParams.pixelSize		= {1920, 1080};
			winParams.pixelPosition = {100, 100};
			winParams.type			= WindowType::WINDOWED;
			winParams.monitor		= 0;

			SDLWindow& mainWindow = GetSoul().Window()->CreateWindow(winParams);

			if (mainWindow.IsValid())
			{
				// Create Vulkan surface from SDL window
				auto vkSurface = mainWindow.CreateVulkanSurface(GetSoul().Raster().InstanceHandle());

				if (vkSurface != 0)
				{
					// Create surface in raster backend
					surfaceEntity_ = GetSoul().Raster().CreateSurface(
						static_cast<NativeSurfaceHandle>(vkSurface),
						{winParams.pixelSize.x, winParams.pixelSize.y});

					// Create render pass
					ShaderSet shaders = {};  // Empty for now, basic clear pass
					renderPassEntity_ = GetSoul().Raster().CreatePass(shaders, [](Entity subPass) {
						// Configure subpass if needed
					});

					// Attach surface to render pass
					GetSoul().Raster().AttachSurface(renderPassEntity_, surfaceEntity_);

					hasValidSurface_ = true;
				}
			}
		}

		GetSoul().Input().AddMousePositionCallback(
			[](double x, double y)
			{
				// TODO: Mouse moved to position (x, y)
			});

		GetSoul().Input().AddMouseButtonCallback(
			[](std::uint32_t button, ButtonState state)
			{
				// TODO: Mouse button event
			});
	}

	void OnUpdate(Frame& current, Frame& previous) override
	{
		GetSoul().Input().Poll();

		if (GetSoul().Window().has_value())
		{
			GetSoul().Window()->Update();

			// Exit if all windows are closed
			if (!GetSoul().Window()->Active())
			{
				GetSoul().SetActive(false);
			}
		}

		UpdateGameLogic(current, previous);

		RenderFrame(current, previous);
	}

	void OnShutdown() override
	{
		// Clean up surfaces before shutdown
		if (hasValidSurface_)
		{
			GetSoul().Raster().DetachSurface(renderPassEntity_, surfaceEntity_);
			GetSoul().Raster().RemoveSurface(surfaceEntity_);
		}
	}

	bool ShouldContinue() const override
	{
		return true;
	}

private:
	void UpdateGameLogic(Frame& current, Frame& previous)
	{
	}

	void RenderFrame(Frame& current, Frame& previous)
	{
		if (!hasValidSurface_)
		{
			return;
		}

		// Execute render pass (clears screen to blue)
		CommandList commandList;
		GetSoul().Raster().ExecutePass(renderPassEntity_, surfaceEntity_, commandList);

		// Present the frame
		GetSoul().Raster().Present();
	}

	Entity surfaceEntity_;
	Entity renderPassEntity_;
	bool hasValidSurface_ = false;
};

std::int32_t main(std::int32_t, char*[])
{
	const synodic::soul::Parameters appParams;
	SDLBackend backend;
	SDLWindowBackend windowBackend(backend);
	SDLInputBackend inputBackend(backend, windowBackend);

	Property<std::uint32_t> threadCount(1);
	PassthroughSchedulerBackend schedulerBackend(threadCount);
	Sample app(appParams, std::move(inputBackend), std::move(windowBackend), schedulerBackend);

	app.Run();

	return 0;
}
