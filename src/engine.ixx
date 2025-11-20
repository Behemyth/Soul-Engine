export module synodic.soul.engine;

import std;

import synodic.cppaste;
export import synodic.soul.core;

import synodic.soul.window;
import synodic.soul.input;
import synodic.soul.gui;
import synodic.soul.raster;
import synodic.soul.render.graph;
import synodic.soul.scheduler;
import synodic.soul.graph;
import synodic.soul.compute;
import synodic.soul.memory;
import synodic.soul.tracer;
import synodic.soul.transput;

namespace synodic
{
	export template<
		typename SchedulerModuleType,
		typename ComputeModuleType,
		typename InputBackendType,
		typename RasterModuleType,
		typename RenderGraphModuleType,
		typename WindowBackendType,
		typename GUIModuleType>
	class Soul final
	{
	public:
		Soul(soul::Parameters& params, InputBackendType inputBackend, WindowBackendType windowBackend) :
			parameters_(params),
			frameTime_(),
			active_(true),
			entityRegistry_(),
			eventRegistry_(),
			schedulerModule_(parameters_.threadCount),
			computeModule_(),
			inputModule_(std::move(inputBackend)),
			rasterModule_(),
			renderGraphModule_(),
			windowModule_(std::move(windowBackend))
		{
			parameters_.engineRefreshRate.AddCallback(
				[this](const std::int32_t value)
				{
					frameTime_ = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(1)) / value;
				});

			parameters_.engineRefreshRate.Update();
		}

		~Soul() = default;

		Soul(const Soul&)	  = delete;
		Soul(Soul&&) noexcept = default;

		Soul& operator=(const Soul&)	 = delete;
		Soul& operator=(Soul&&) noexcept = default;

		SchedulerModuleType& Scheduler()
		{
			return schedulerModule_;
		}

		ComputeModuleType& Compute()
		{
			return computeModule_;
		}

		InputBackendType& Input()
		{
			return inputModule_;
		}

		RasterModuleType& Raster()
		{
			return rasterModule_;
		}

		RenderGraphModuleType& RenderGraph()
		{
			return renderGraphModule_;
		}

		std::optional<WindowBackendType>& Window()
		{
			return windowModule_;
		}

		std::optional<GUIModuleType>& GUI()
		{
			return guiModule_;
		}

		EntityRegistry& Entities()
		{
			return entityRegistry_;
		}

		EventRegistry& Events()
		{
			return eventRegistry_;
		}

		soul::Parameters& Parameters()
		{
			return parameters_;
		}

		bool IsActive() const
		{
			return active_;
		}

		void SetActive(bool active)
		{
			active_ = active;
		}

		std::chrono::nanoseconds GetFrameTime() const
		{
			return frameTime_;
		}

	private:
		soul::Parameters& parameters_;
		std::chrono::nanoseconds frameTime_;
		bool active_;

		EntityRegistry entityRegistry_;
		EventRegistry eventRegistry_;
		SchedulerModuleType schedulerModule_;
		ComputeModuleType computeModule_;
		InputBackendType inputModule_;
		RasterModuleType rasterModule_;
		RenderGraphModuleType renderGraphModule_;

		// Potentially empty modules
		std::optional<WindowBackendType> windowModule_;
		std::optional<GUIModuleType> guiModule_;
	};

	namespace soul
	{

		export template<
			typename SchedulerModuleType,
			typename ComputeModuleType,
			typename InputBackendType,
			typename RasterModuleType,
			typename RenderGraphModuleType,
			typename WindowBackendType,
			typename GUIModuleType>
		class App
		{
		public:
			App(const Parameters& params, InputBackendType inputBackend, WindowBackendType windowBackend) :
				parameters_(params),
				soul_(parameters_, std::move(inputBackend), std::move(windowBackend)),
				currentFrame_(),
				previousFrame_()
			{
			}

			virtual ~App() = default;

			void Run()
			{
				OnInit();

				while (soul_.IsActive() && ShouldContinue())
				{
					// Advance frame
					previousFrame_ = Frame(currentFrame_, previousFrame_);
					currentFrame_  = Frame(previousFrame_, currentFrame_);

					OnUpdate(currentFrame_, previousFrame_);
				}

				OnShutdown();
			}

		protected:

			// Called once before the main loop starts
			virtual void OnInit()
			{
			}

			// Called every frame - user defines the update logic
			virtual void OnUpdate(Frame& current, Frame& previous)
			{
			}

			// Called once after the main loop ends
			virtual void OnShutdown()
			{
			}

			// Override to define custom exit conditions (default: always continue)
			virtual bool ShouldContinue() const
			{
				return true;
			}

			Soul<
				SchedulerModuleType,
				ComputeModuleType,
				InputBackendType,
				RasterModuleType,
				RenderGraphModuleType,
				WindowBackendType,
				GUIModuleType>&
				GetSoul()
			{
				return soul_;
			}

			const Soul<
				SchedulerModuleType,
				ComputeModuleType,
				InputBackendType,
				RasterModuleType,
				RenderGraphModuleType,
				WindowBackendType,
				GUIModuleType>&
				GetSoul() const
			{
				return soul_;
			}

			Parameters& GetParameters()
			{
				return parameters_;
			}

			const Parameters& GetParameters() const
			{
				return parameters_;
			}

			Frame& GetCurrentFrame()
			{
				return currentFrame_;
			}

			Frame& GetPreviousFrame()
			{
				return previousFrame_;
			}

		private:
			Parameters parameters_;
			Soul<
				SchedulerModuleType,
				ComputeModuleType,
				InputBackendType,
				RasterModuleType,
				RenderGraphModuleType,
				WindowBackendType,
				GUIModuleType>
				soul_;

			Frame currentFrame_;
			Frame previousFrame_;
		};
	}
}
