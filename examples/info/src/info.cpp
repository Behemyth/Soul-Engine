import std;
import synodic.soul.core;
import synodic.soul.engine;
import synodic.soul.scheduler;

import synodic.soul.window.backend.mock;
import synodic.soul.input.backend.mock;
import synodic.soul.raster.backend.mock;
import synodic.soul.gui.backend.standard;
import synodic.soul.render.graph.backend.standard;
import synodic.soul.compute.backend.mock;

using InfoApp = synodic::soul::App<
	SchedulerModule,
	MockBackend,
	MockInputBackend,
	MockRasterBackend,
	StandardRenderGraphBackend,
	MockWindowBackend,
	StandardGUIBackend>;

class Info : public InfoApp
{
public:
	explicit Info(
		const synodic::soul::Parameters& params,
		MockInputBackend inputBackend,
		MockWindowBackend windowBackend) :
		InfoApp(params, std::move(inputBackend), std::move(windowBackend))
	{
	}

protected:
	void OnInit() override
	{
		GatherSystemInfo();
		DisplayInfo();

		// Signal to exit immediately
		GetSoul().SetActive(false);
	}

	// Info app doesn't need OnUpdate since it exits after OnInit
	// OnUpdate won't be called because IsActive() is false

private:
	void GatherSystemInfo()
	{
		// TODO: Gather CPU info
		// TODO: Gather GPU info
		// TODO: Gather memory info
		// TODO: Gather available backends

		// For now, just gather basic parameters
		const auto& params = GetParameters();
		threadCount_	   = params.threadCount;
		refreshRate_	   = params.engineRefreshRate;
	}

	void DisplayInfo()
	{
		// TODO: Format and display system information
		std::print("Soul Engine Information\n");
		std::print("=======================\n");
		std::print("Thread Count: {}\n", threadCount_);
		std::print("Refresh Rate: {} Hz\n", refreshRate_);
		std::print("\n");
	}

	std::uint32_t threadCount_ = 0;
	std::int32_t refreshRate_  = 0;
};

std::int32_t main(std::int32_t, char*[])
{
	const synodic::soul::Parameters appParams;
	MockInputBackend inputBackend;
	MockWindowBackend windowBackend;
	Info app(appParams, std::move(inputBackend), std::move(windowBackend));

	app.Run();

	return 0;
}
