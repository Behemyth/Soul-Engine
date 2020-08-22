from conans import ConanFile, CMake

class SoulEngine(ConanFile):

	name = "soul-engine"
	version = "0.1.0"
	author = "Synodic Software"
	license = "GPLv3"
	url = "https://github.com/Synodic-Software/Soul-Engine"
	description = "Soul Engine is a real-time visualization and simulation engine built on the back of CUDA, OpenCL, and Vulkan."

	settings = "os", "compiler", "build_type", "arch"
	options = {"shared": [True, False]}
	default_options = {"shared": False}

	requires = "doctest/2.4.0", "cppaste/0.1.0", "tracer/0.1.0", "acceleration-structure/0.1.0", "glm/0.9.9.8"
	generators = "cmake_find_package"

	scm = {
         "type": "git",
         "url": "auto",
         "revision": "auto",
	}

	def build(self):
		cmake = CMake(self)
		cmake.configure()
		cmake.build()

	def package(self):
		self.copy("*.h", src="src", dst="include")
		self.copy("*.lib", dst="lib", keep_path=False)
		self.copy("*.dll", dst="bin", keep_path=False)
		self.copy("*.so", dst="lib", keep_path=False)
		self.copy("*.dylib", dst="lib", keep_path=False)
		self.copy("*.a", dst="lib", keep_path=False)

	def package_info(self):
		self.cpp_info.libs = ["soul-engine"]

		# If the the package is in editable mode, forward the library path
		if not self.in_local_cache:
			self.cpp_info.includedirs = ["src"]
			self.cpp_info.libdirs = ["build"]