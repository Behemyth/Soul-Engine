export module synodic.soul.window.backend.mock;

import std;

import synodic.soul.window;

// Mock window implementation (no actual windowing)
export class MockWindow : public Window<MockWindow>
{
public:
	explicit MockWindow(const WindowParameters& params) :
		Window<MockWindow>(params)
	{
	}

	~MockWindow() = default;

	MockWindow(const MockWindow&)	  = delete;
	MockWindow(MockWindow&&) noexcept = default;

	MockWindow& operator=(const MockWindow&)	 = delete;
	MockWindow& operator=(MockWindow&&) noexcept = default;

	// Mock-specific: no native handle
	bool IsValid() const
	{
		return true;
	}
};

// Mock Window Backend
export class MockWindowBackend : public WindowModule<MockWindow>
{
public:
	MockWindowBackend()			  = default;
	~MockWindowBackend() override = default;

	MockWindowBackend(const MockWindowBackend&) = delete;
	MockWindowBackend(MockWindowBackend&&)		= default;

	MockWindowBackend& operator=(const MockWindowBackend&)	   = delete;
	MockWindowBackend& operator=(MockWindowBackend&&) noexcept = default;

	void Update() override
	{
		// Mock: do nothing
	}

	std::span<const char*> GetRasterExtensions() override
	{
		return {};
	}

protected:
	void OnWindowCreated(MockWindow& window) override
	{
		// Mock: no actual window creation
	}
};
