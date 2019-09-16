#pragma once

enum class PlatformID { Windows, OSX, Linux, Android, Unknown };

class Platform {

public:

	constexpr static PlatformID GetPlatform();
	constexpr static bool IsDesktop();

private:

	#if defined(__linux__) && !defined(__ANDROID__)

	constexpr static PlatformID platform_ = PlatformID::Linux;

	#elif defined(_WIN64)

	constexpr static PlatformID platform_ = PlatformID::Windows;

	#elif defined(__APPLE__)

	constexpr static PlatformID platform_ = PlatformID::OSX;

	#elif defined(__ANDROID__) 

	constexpr static PlatformID platform_ = PlatformID::Android;

	#else

	constexpr static PlatformID platform_ = PlatformID::Unknown;

	#endif

};

constexpr PlatformID Platform::GetPlatform() {
	return platform_;
}

constexpr bool Platform::IsDesktop() {
	return platform_ == PlatformID::Linux ||
		platform_ == PlatformID::Windows ||
		platform_ == PlatformID::OSX;
}