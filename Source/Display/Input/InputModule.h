#pragma once


#include "Core/Interface/Module/Module.h"
#include "InputSet.h"
#include "Button.h"

#include <memory>
#include <functional>


class Window;


class InputModule : Module<InputModule> {

public:

	InputModule() = default;
	virtual ~InputModule() = default;

	InputModule(const InputModule&) = delete;
	InputModule(InputModule&&) noexcept = default;

	InputModule& operator=(const InputModule&) = delete;
	InputModule& operator=(InputModule&&) noexcept = default;


	virtual bool Poll() = 0;
	virtual void Listen(Window&) = 0;

	void AddMousePositionCallback(std::function<void(double, double)>);
	void AddMouseButtonCallback(std::function<void(uint, ButtonState)>);

	// Factory
	static std::unique_ptr<InputModule> CreateModule();

protected:

	// TODO: Implement proper InputSet
	InputSet globalInputSet_;

	std::vector<std::function<void(double, double)>> mousePositionCallbacks_;
	std::vector<std::function<void(uint, ButtonState)>> mouseButtonCallbacks_;

};
