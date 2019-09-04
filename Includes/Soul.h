#pragma once

#include "SoulParameters.h"

#include <memory>
#include <chrono>


class WindowParameters;
class Window;
class Frame;

class SchedulerModule;
class ComputeModule;
class InputModule;
class WindowModule;
class RasterModule;
class RenderGraphModule;
class GUIModule;

class EntityRegistry;
class EventRegistry;

class Implementation;

class Soul final{

public:


	Soul(SoulParameters&);
	~Soul() = default;

	Soul(const Soul&) = delete;
	Soul(Soul&&) noexcept = default;

	Soul& operator=(const Soul&) = delete;
	Soul& operator=(Soul&&) noexcept = default;

	void Init();
	void CreateWindow(WindowParameters&);


private:

	void Run();

	//Pipeline Functions
	void Process(Frame&, Frame&);
	void Update(Frame&, Frame&);
	void Render(Frame&, Frame&);


	//State Functions

	void Warmup();

	void EarlyFrameUpdate(Frame&, Frame&);
	void LateFrameUpdate(Frame&, Frame&);
	void EarlyUpdate(Frame&, Frame&);
	void LateUpdate(Frame&, Frame&);

	SoulParameters& parameters_;

	std::chrono::nanoseconds frameTime_;
	bool active_;


	//services and modules	
	std::shared_ptr<EntityRegistry> entityRegistry_;
	std::shared_ptr<EventRegistry> eventRegistry_;

	std::shared_ptr<SchedulerModule> schedulerModule_;
	std::shared_ptr<ComputeModule> computeModule_;
	std::shared_ptr<InputModule> inputModule_;
	std::shared_ptr<WindowModule> windowModule_;
	std::shared_ptr<RasterModule> rasterModule_;
	std::shared_ptr<RenderGraphModule> renderGraphModule_;
	std::shared_ptr<GUIModule> guiModule_;


};
