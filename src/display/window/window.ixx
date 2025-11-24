export module synodic.soul.window;

import synodic.soul.core;

export import :monitor;

export template<typename Derived>
class Window
{
public:
	Window(const WindowParameters& params) :
		layout_(),
		surface_(),
		windowParams_(params)
	{
	}

	~Window() = default;

	Window(const Window&)	  = delete;
	Window(Window&&) noexcept = default;

	Window& operator=(const Window&)	 = delete;
	Window& operator=(Window&&) noexcept = default;

	WindowParameters& Parameters()
	{
		return windowParams_;
	}

	const WindowParameters& Parameters() const
	{
		return windowParams_;
	}

	Entity Surface()
	{
		return surface_;
	}

	Entity Layout()
	{
		return layout_;
	}

protected:
	Entity layout_;
	Entity surface_;
	WindowParameters windowParams_;

	Derived& GetDerived()
	{
		return static_cast<Derived&>(*this);
	}

	const Derived& GetDerived() const
	{
		return static_cast<const Derived&>(*this);
	}
};

export template<typename WindowType>
class WindowModule
{
public:
	WindowModule() :
		active_(false)
	{
	}

	virtual ~WindowModule() = default;

	WindowModule(const WindowModule&)	  = delete;
	WindowModule(WindowModule&&) noexcept = default;

	WindowModule& operator=(const WindowModule&)	 = delete;
	WindowModule& operator=(WindowModule&&) noexcept = default;

	WindowType& CreateWindow(const WindowParameters& params)
	{
		auto& window = windows_.emplace_back(params);
		active_		 = true;

		OnWindowCreated(window);

		return window;
	}

	void DestroyWindow(std::size_t index)
	{
		if (index < windows_.size())
		{
			OnWindowDestroyed(windows_[index]);
			windows_.erase(windows_.begin() + index);

			if (windows_.empty())
			{
				active_ = false;
			}
		}
	}

	WindowType* GetWindow(std::size_t index)
	{
		return index < windows_.size() ? &windows_[index] : nullptr;
	}

	WindowType& GetPrimaryWindow()
	{
		return windows_.front();
	}

	std::span<WindowType> GetWindows()
	{
		return windows_;
	}

	std::size_t GetWindowCount() const
	{
		return windows_.size();
	}

	virtual void Update() = 0;

	virtual bool Active() const
	{
		return active_;
	}

	virtual std::span<const char*> GetRasterExtensions() = 0;

	// Generic surface creation: raster backend provides a callback that receives the native window handle
	// This keeps the window module agnostic of specific graphics APIs
	template<typename SurfaceCreator>
	auto CreateSurface(WindowType& window, SurfaceCreator&& creator)
	{
		return creator(window.GetNativeHandle());
	}

protected:
	// Override these for backend-specific window lifecycle
	virtual void OnWindowCreated(WindowType& window)
	{
	}

	virtual void OnWindowDestroyed(WindowType& window)
	{
	}

	std::vector<WindowType> windows_;
	bool active_;
};
