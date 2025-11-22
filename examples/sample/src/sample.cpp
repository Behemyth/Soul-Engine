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

import synodic.soul.backend.sdl;

using SampleApp = synodic::soul::App<
	SchedulerModule,
	MockBackend,
	SDLInputBackend,
	VulkanRasterBackend,
	StandardRenderGraphBackend,
	SDLWindowBackend,
	StandardGUIBackend>;

class Sample : public SampleApp
{
public:
	explicit Sample(
		const synodic::soul::Parameters& params,
		SDLInputBackend inputBackend,
		SDLWindowBackend windowBackend) :
		SampleApp(params, std::move(inputBackend), std::move(windowBackend))
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

			if (mainWindow.GetNativeHandle())
			{
				// Window created successfully
			}
		}

		GetSoul().Input().AddMousePositionCallback([](double x, double y) {
			// TODO: Mouse moved to position (x, y)
		});

		GetSoul().Input().AddMouseButtonCallback([](std::uint32_t button, ButtonState state) {
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
		if (GetSoul().Window().has_value())
		{
			auto windows = GetSoul().Window()->GetWindows();
			for (auto& window: windows)
			{
				// TODO: Render to each window
			}
		}
	}
};

std::int32_t main(std::int32_t, char*[])
{
	const synodic::soul::Parameters appParams;
	SDLBackend backend;
	SDLWindowBackend windowBackend(backend);
	SDLInputBackend inputBackend(backend, windowBackend);
	Sample app(appParams, std::move(inputBackend), std::move(windowBackend));

	app.Run();

	return 0;
}
