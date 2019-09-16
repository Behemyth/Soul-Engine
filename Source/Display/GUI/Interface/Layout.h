#pragma once

#include "Widget.h"

#include <vector>
#include <memory>

class Layout : public Widget
{

public:

	Layout() = default;
	virtual ~Layout() = default;

	Layout(const Layout&) = delete;
	Layout(Layout&&) noexcept = default;

	Layout& operator=(const Layout&) = delete;
	Layout& operator=(Layout&&) noexcept = default;

	template<typename T, typename...Args>
	T& AddLayout(Args&&...);

	template<typename T, typename...Args>
	T& AddWidget(Args&&...);

protected:

	std::vector<std::unique_ptr<Widget>> widgets_;

};

template<typename T, typename...Args>
T& Layout::AddLayout(Args&&... args) {
	widgets_.push_back(std::make_unique<T>(std::forward<Args>(args)...));
	return *static_cast<T*>(widgets_.back().get());
}

template<typename T, typename...Args>
T& Layout::AddWidget(Args&&... args) {
	widgets_.push_back(std::make_unique<T>(std::forward<Args>(args)...));
	return *static_cast<T*>(widgets_.back().get());
}
