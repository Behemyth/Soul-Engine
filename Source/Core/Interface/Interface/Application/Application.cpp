#include "SoulApplication.h"
#include "Soul.h"

SoulApplication::SoulApplication(SoulParameters params) :
	hasControl(true),
	parameters(params),
	soul(parameters)
{
}

void SoulApplication::CreateWindow(WindowParameters& params) {

	soul.CreateWindow(params);

}


void SoulApplication::Run() {

	CheckParameters();

	//EventManager::Listen("Input", "ESCAPE", [](keyState state) {
	//	if (state == RELEASE) {
	//		SoulSignalClose();
	//	}
	//});

	soul.Init();

}

void SoulApplication::CheckParameters() {

	//TODO
	
}
