export module synodic.soul.input:input_module;

import std;
import synodic.soul.core;
import :input_set;
import :button;

export class InputModule : Module<InputModule>
{
public:
	InputModule()			= default;
	~InputModule() override = default;

	InputModule(const InputModule&)		= delete;
	InputModule(InputModule&&) noexcept = default;

	InputModule& operator=(const InputModule&)	   = delete;
	InputModule& operator=(InputModule&&) noexcept = default;

	virtual bool Poll() = 0;
	// virtual void Listen(Window&) = 0;

	void AddMousePositionCallback(std::function<void(double, double)>);
	void AddMouseButtonCallback(std::function<void(std::uint32_t, ButtonState)>);

	// Factory
	static std::unique_ptr<InputModule> CreateModule();

protected:
	// TODO: Implement proper InputSet
	InputSet globalInputSet_;

	std::vector<std::function<void(double, double)>> mousePositionCallbacks_;
	std::vector<std::function<void(std::uint32_t, ButtonState)>> mouseButtonCallbacks_;
};

inline void InputModule::AddMousePositionCallback(std::function<void(double, double)> callback)
{
	mousePositionCallbacks_.push_back(callback);
}

inline void InputModule::AddMouseButtonCallback(std::function<void(std::uint32_t, ButtonState)> callback)
{
	mouseButtonCallbacks_.push_back(callback);
}

// TODO: There will only ever be one DisplayModule system per Soul application. This will need to be moved to the build
// system per platform
inline std::unique_ptr<InputModule> InputModule::CreateModule()
{
	return nullptr;
}
