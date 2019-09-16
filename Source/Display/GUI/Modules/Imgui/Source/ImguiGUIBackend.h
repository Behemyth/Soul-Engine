#pragma once

#include "Display/GUI/GUIModule.h"

class ImguiBackend final : public GUIModule {

public:

	ImguiBackend(std::shared_ptr<InputModule>&,
		std::shared_ptr<WindowModule>&,
		std::shared_ptr<RenderGraphModule>&);
	~ImguiBackend() override;

	ImguiBackend(const ImguiBackend&) = delete;
	ImguiBackend(ImguiBackend&&) noexcept = default;

	ImguiBackend& operator=(const ImguiBackend&) = delete;
	ImguiBackend& operator=(ImguiBackend&&) noexcept = default;


	void Update(std::chrono::nanoseconds) override;

private:

	void ConvertRetained();

	std::shared_ptr<InputModule> inputModule_;
	std::shared_ptr<WindowModule> windowModule_;
	std::shared_ptr<RenderGraphModule> renderGraphModule_;

};

