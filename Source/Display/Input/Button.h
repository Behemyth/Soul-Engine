#pragma once

#include "Core/Utility/Timer/Timer.h"

enum class ButtonState { PRESS, REPEAT, RELEASE, OPEN };

class Button {

public:

	Button();

	float timeToRepeat; //in milliseconds
	Timer sincePress;


	ButtonState state;

};
