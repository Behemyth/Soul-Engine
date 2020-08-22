#pragma once

#include <functional>

#include <glm/vec4.hpp>
#include "synodic/tracer.h"
#include "synodic/acceleration_structure.h"

namespace synodic
{

	struct SoulParameters
	{
	};

	class Soul final
	{

	public:
		explicit Soul(const SoulParameters&);
		~Soul() = default;

		void Run();
		void Tick(uint32_t count = 1);
		void Poll();

		void AddTracerJob(
			const TracerJob& tracerJob,
			const std::function<void(std::span<glm::vec4>)>& callback) const;

	private:
		class Modules
		{
		public:
			explicit Modules(Tracer&);
			~Modules() = default;

			Tracer& tracer;
		};

		[[nodiscard]] Modules InitModules() const;

		Modules modules_;
		bool active_;
	};

}