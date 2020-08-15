#pragma once

#include "synodic/tracer.h"


namespace synodic
{
	
	class Soul final
	{
				
	public:
		Soul();
		~Soul() = default;
		
	private:
		class Modules
		{
		public:
			explicit Modules(std::unique_ptr<Tracer>);
			~Modules() = default;

		};

		[[nodiscard]] Modules InitModules() const;

		Modules modules_;
	};

}