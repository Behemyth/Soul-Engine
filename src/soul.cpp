#include "soul.h"

#include "synodic/composition/module.h"
#include "synodic/composition/package.h"
#include "synodic/cpu_tracer.h"
#include "synodic/lbvh.h"

namespace synodic
{

	Soul::Modules Soul::InitModules() const
	{

		const ModuleParameters accelerationParams;
		const auto accelerationModule =
			synodic::CreateModule<AccelerationStructureModule, LBVHModule>(accelerationParams);

		const ModuleParameters tracerParams;
		const auto tracerModule =
			synodic::CreateModule<TracerModule, CPUTracerModule>(tracerParams);

		const auto package = Package(accelerationModule, tracerModule);
		return package.Create<Modules>();
	}

	Soul::Modules::Modules(Tracer& tracerIn) : tracer(tracerIn)
	{
	}

	Soul::Soul(const SoulParameters& soulParameters) : modules_(InitModules()), active_(false)
	{
	}


	// Soul::Soul(SoulParameters& params) :
	//	parameters_(params),
	//	frameTime_(),
	//	active_(true),
	//	entityRegistry_(new EntityRegistry()),
	//	eventRegistry_(new EventRegistry()),
	//	schedulerModule_(SchedulerModule::CreateModule(parameters_.threadCount)),
	//	computeModule_(ComputeModule::CreateModule()),
	//	inputModule_(InputModule::CreateModule()),
	//	windowModule_(WindowModule::CreateModule(inputModule_)),
	//	rasterModule_(RasterModule::CreateModule(schedulerModule_, entityRegistry_, windowModule_)),
	//	renderGraphModule_(
	//		RenderGraphModule::CreateModule(rasterModule_, schedulerModule_, entityRegistry_)),
	//	guiModule_(GUIModule::CreateModule(inputModule_, windowModule_, renderGraphModule_))
	//{
	//	parameters_.engineRefreshRate.AddCallback([this](const int value)
	//	{
	//		frameTime_ =
	// std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(1))
	///
	// value;
	//	});
	//
	//	//flush parameters_ with new callbacks
	//	parameters_.engineRefreshRate.Update();
	//}
	//
	//
	///////////////////////////Core/////////////////////////////////
	//
	// void Soul::Process(Frame& oldFrame, Frame& newFrame) {
	//
	//	EarlyFrameUpdate(oldFrame, newFrame);
	//
	//}
	//
	// void Soul::Update(Frame& oldFrame, Frame& newFrame) {
	//
	//	EarlyUpdate(oldFrame, newFrame);
	//
	//	if (newFrame.Dirty()) {
	//
	//		//	for (auto& scene : scenes) {
	//		//		scene->Build(engineRefreshRate);
	//		//	}
	//		//
	//		//	for (auto const& scene : scenes){
	//		//		PhysicsEngine::Process(scene);
	//		//	}
	//
	//	}
	//
	//	LateUpdate(oldFrame, newFrame);
	//
	//}
	//
	// void Soul::Render(Frame& oldFrame, Frame& newFrame) {
	//
	//	LateFrameUpdate(oldFrame, newFrame);
	//
	//	if (newFrame.Dirty()) {
	//
	//		//	//RayEngine::Instance().Process(*scenes[0], engineRefreshRate);
	//		renderGraphModule_->Execute();
	//		rasterModule_->Present();
	//
	//	}
	//
	//}

	// void Soul::EarlyFrameUpdate(Frame& oldFrame, Frame& newFrame)
	//{
	//
	//	eventRegistry_->Emit("Update"_hashed, "EarlyFrame"_hashed);
	//
	//}
	//
	// void Soul::LateFrameUpdate(Frame& oldFrame, Frame& newFrame)
	//{
	//
	//	eventRegistry_->Emit("Update"_hashed, "LateFrame"_hashed);
	//
	//	//Additional Poll as the inner update may still have taken some time
	//	inputModule_->Poll();
	//
	//	//TODO: Is Update even needed for windowModule_?
	//	//Update the window state as late as possible before rendering
	//	//windowModule_->Update();
	//
	//	if (windowModule_->Active()) {
	//
	//		guiModule_->Update(frameTime_);
	//
	//	}
	//
	//}
	//
	// void Soul::EarlyUpdate(Frame& oldFrame, Frame& newFrame)
	//{
	//
	//	eventRegistry_->Emit("Update"_hashed, "Early"_hashed);
	//
	//	//Poll the keys as late as possible before an update
	//	newFrame.Dirty(inputModule_->Poll());
	//	active_ = windowModule_->Active();
	//
	//	//Update the engine cameras
	//	//RayEngine::Instance().Update();
	//
	//	//pull cameras into jobs
	//	eventRegistry_->Emit("Update"_hashed, "Job Cameras"_hashed);
	//
	//}
	//
	// void Soul::LateUpdate(Frame& oldFrame, Frame& newFrame)
	//{
	//
	//	eventRegistry_->Emit("Update"_hashed, "Late"_hashed);
	//
	//
	//}
	//
	// void Soul::Init()
	//{
	//
	//	Warmup();
	//
	//	//TODO: Remove as it is temporary
	//	Run();
	//
	//}
	//
	// void Soul::CreateWindow(WindowParameters& params) {
	//
	//	windowModule_->CreateWindow(params, rasterModule_);
	//
	//}

	void Soul::Run()
	{

		// auto currentTime = std::chrono::time_point_cast<std::chrono::nanoseconds>(
		//	std::chrono::high_resolution_clock::now());
		// auto nextTime = currentTime + frameTime_;

		// FramePipeline<3> framePipeline {
		//	schedulerModule_,
		//	{[this](Frame& oldFrame, Frame& newFrame) {
		//		 Process(oldFrame, newFrame);
		//	 },
		//	 [this](Frame& oldFrame, Frame& newFrame) {
		//		 Update(oldFrame, newFrame);
		//	 },
		//	 [this](Frame& oldFrame, Frame& newFrame) {
		//		 Render(oldFrame, newFrame);
		//	 }}};

		active_ = true;
		while(active_)
		{

			/*currentTime = nextTime;
			nextTime	= currentTime + frameTime_;

			framePipeline.Execute(frameTime_);

			schedulerModule_->YieldUntil(nextTime);*/
		}
	}

	void Soul::Tick(const uint32_t count)
	{
		for(uint32_t index = 0; index < count; ++index)
		{
		}
	}

	void Soul::AddTracerJob(
		const TracerJob& tracerJob,
		const std::function<void(std::span<glm::vec4>)>& callback) const
	{
		modules_.tracer.AddJob(tracerJob, callback);
	}

}