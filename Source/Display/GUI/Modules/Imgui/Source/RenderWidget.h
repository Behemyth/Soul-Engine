#pragma once

#include "ImguiWidget.h"


class RenderWidget : public ImguiWidget
{

public:

	RenderWidget() = default;
	~RenderWidget() override = default;

	RenderWidget(const RenderWidget&) = delete;
	RenderWidget(RenderWidget&&) noexcept = default;

	RenderWidget& operator=(const RenderWidget&) = delete;
	RenderWidget& operator=(RenderWidget&&) noexcept = default;

};

