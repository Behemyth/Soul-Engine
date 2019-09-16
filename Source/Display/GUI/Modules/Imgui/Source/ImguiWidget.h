#pragma once

#include "Display/GUI/Widget.h"


class ImguiWidget : public Widget {

public:

	ImguiWidget() = default;
	~ImguiWidget() override = default;

	ImguiWidget(const ImguiWidget&) = delete;
	ImguiWidget(ImguiWidget&&) noexcept = default;

	ImguiWidget& operator=(const ImguiWidget&) = delete;
	ImguiWidget& operator=(ImguiWidget&&) noexcept = default;


};

