#pragma once

#include "Soul.h"
#include "SoulParameters.h"
#include "WindowParameters.h"

class SoulApplication {

public:

	SoulApplication(SoulParameters = SoulParameters());
	virtual ~SoulApplication() = default;

	void CreateWindow(WindowParameters&);

	void Run();

protected:

	bool hasControl;
	SoulParameters parameters;

private:

	void CheckParameters();

	Soul soul;

};
