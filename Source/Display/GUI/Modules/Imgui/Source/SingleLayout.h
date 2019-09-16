#pragma once

#include "ImguiLayout.h"


//A Layout with a single widget slot, covering its entire screenspace
class SingleLayout : public ImguiLayout
{

public:

	SingleLayout() = default;
	~SingleLayout() override = default;

	SingleLayout(const SingleLayout&) = delete;
	SingleLayout(SingleLayout&&) noexcept = default;

	SingleLayout& operator=(const SingleLayout&) = delete;
	SingleLayout& operator=(SingleLayout&&) noexcept = default;


};

