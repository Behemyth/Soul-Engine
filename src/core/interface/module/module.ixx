export module synodic.soul.core:module;

//Common interface for modules.
export template<typename T>
class Module {

public:

	virtual ~Module() = default;

	Module(const Module&) = delete;
	Module(Module&&) noexcept = default;

	Module& operator=(const Module&) = delete;
	Module& operator=(Module&&) noexcept = default;


protected:

	Module() = default;


};
