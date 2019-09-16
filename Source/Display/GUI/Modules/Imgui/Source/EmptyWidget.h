#pragma once

#include "ImguiWidget.h"


class EmptyWidget : public ImguiWidget
{

public:

	EmptyWidget() = default;
	~EmptyWidget() = default;

	EmptyWidget(const EmptyWidget&) = delete;
	EmptyWidget(EmptyWidget&&) noexcept = default;

	EmptyWidget& operator=(const EmptyWidget&) = delete;
	EmptyWidget& operator=(EmptyWidget&&) noexcept = default;


};

