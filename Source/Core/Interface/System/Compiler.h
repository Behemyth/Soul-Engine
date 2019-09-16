#pragma once

enum class CompilerID { VisualStudio, GCC, Clang, Emscripten, Unknown };

class Compiler {

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