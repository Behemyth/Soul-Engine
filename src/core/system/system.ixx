export module synodic.soul.engine:core.system;

export enum class PlatformID { Windows, OSX, Linux, Android, Unknown };

export class Platform {

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

export enum class CompilerID { VisualStudio, GCC, Clang, Emscripten, Unknown };

export class Compiler {

public:

	constexpr static CompilerID GetCompiler();
	constexpr static bool Debug();


private:

	#if defined(_MSC_VER)

	constexpr static CompilerID compiler_ = CompilerID::VisualStudio;

	#elif defined(__GNUC__)

	constexpr static CompilerID compiler_ = CompilerID::GCC;

	#elif defined(__clang__)

	constexpr static CompilerID compiler_ = CompilerID::Clang;

	#elif defined(__EMSCRIPTEN__)

	constexpr static CompilerID compiler_ = CompilerID::Emscripten;

	#else

	constexpr static CompilerID compiler_ = CompilerID::Unknown;

	#endif

	//TODO: Update debug macro
	#if defined(NDEBUG)

	constexpr static bool debug_ = false;

	#else

	constexpr static bool debug_ = true;

	#endif


};

constexpr CompilerID Compiler::GetCompiler() {
	return compiler_;
}

constexpr bool Compiler::Debug() {
	return debug_;
}
