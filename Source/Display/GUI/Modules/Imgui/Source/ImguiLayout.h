#pragma once

#include "Display/GUI/Layout.h"


class ImguiLayout : public Layout
{

public:

	ImguiLayout() = default;
	~ImguiLayout() override = default;

	ImguiLayout(const ImguiLayout&) = delete;
	ImguiLayout(ImguiLayout&&) noexcept = default;

	ImguiLayout& operator=(const ImguiLayout&) = delete;
	ImguiLayout& operator=(ImguiLayout&&) noexcept = default;


};
