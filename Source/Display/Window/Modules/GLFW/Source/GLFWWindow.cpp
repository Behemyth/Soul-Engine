#include "GLFWWindow.h"


#include "GLFWMonitor.h"
#include "Render/Raster/RasterModule.h"
#include "Render/Raster/Modules/Vulkan/VulkanRasterBackend.h"

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <any>

GLFWWindow::GLFWWindow(const WindowParameters& params,
	GLFWMonitor& monitor,
	std::shared_ptr<RasterModule> rasterModule,
	bool master):
	Window(params),
	rasterModule_(rasterModule), master_(master)
{
	int redBits, greenBits, blueBits, refreshRate;
	monitor.ColorBits(redBits, greenBits, blueBits);
	monitor.RefreshRate(refreshRate);

	glfwWindowHint(GLFW_RED_BITS, redBits);
	glfwWindowHint(GLFW_GREEN_BITS, greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, refreshRate);

	// TODO Cleanup and implement more features

	// defaulted params
	GLFWmonitor* fullscreenMonitor = nullptr;

	// specific window type creation settings, each setting is global, so all should be set in each
	// possibility
	if (windowParams_.type == WindowType::FULLSCREEN) {

		glfwWindowHint(GLFW_RESIZABLE, false);
		glfwWindowHint(GLFW_DECORATED, false);

		fullscreenMonitor = monitor.monitor_;
	}
	else if (windowParams_.type == WindowType::WINDOWED) {

		glfwWindowHint(GLFW_RESIZABLE, true);
		glfwWindowHint(GLFW_DECORATED, true);
	}
	else if (windowParams_.type == WindowType::BORDERLESS) {

		glfwWindowHint(GLFW_RESIZABLE, false);
		glfwWindowHint(GLFW_DECORATED, false);
	}
	else {

		glfwWindowHint(GLFW_RESIZABLE, false);
		glfwWindowHint(GLFW_DECORATED, true);
	}

	context_ = glfwCreateWindow(windowParams_.pixelSize.x, windowParams_.pixelSize.y,
		windowParams_.title.c_str(), fullscreenMonitor, nullptr);

	assert(context_);


	// GLFW guarantees Vulkan support. Grab the surface on the sly
	std::any surface;
	{
		
		const auto vulkanRasterModule =
			std::static_pointer_cast<VulkanRasterBackend>(rasterModule_);
		VkSurfaceKHR castSurface;
		
		// guaranteed to use GLFW if using Vulkan
		const VkResult error = glfwCreateWindowSurface(static_cast<VkInstance>(vulkanRasterModule->Instance().Handle()),
				context_, nullptr, &castSurface);

		assert(error == VK_SUCCESS);

		surface = static_cast<vk::SurfaceKHR>(castSurface);
	}

	
	surface_ = rasterModule_->CreateSurface(surface, windowParams_.pixelSize);
}

GLFWWindow::~GLFWWindow()
{

	rasterModule_->RemoveSurface(surface_);

	glfwDestroyWindow(context_);
	
}

void GLFWWindow::FrameBufferResize(int x, int y)
{

	windowParams_.pixelSize = {x, y};
	rasterModule_->UpdateSurface(surface_, windowParams_.pixelSize);
}

GLFWwindow* GLFWWindow::Context() const
{

	return context_;
}

bool GLFWWindow::Master() const
{

	return master_;
}
