export module synodic.soul.core:component;

export class Component
{

public:

	Component() = default;
	~Component() = default;

	Component(const Component&) = delete;
	Component(Component&&) noexcept = default;

	Component& operator=(const Component&) = delete;
	Component& operator=(Component&&) noexcept = default;


};
