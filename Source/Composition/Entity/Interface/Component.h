#pragma once

#include "Core/Utility/Template/CRTP.h"


class EntityManager;

class Component 
{

public:

	Component() = default;
	~Component() = default;

	Component(const Component&) = delete;
	Component(Component&&) noexcept = default;

	Component& operator=(const Component&) = delete;
	Component& operator=(Component&&) noexcept = default;


};