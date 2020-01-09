#pragma once

#include "Control.hpp"

namespace inl::gui {


class Layout : public Control {
public:
	virtual ~Layout() = default;

	// Layout update
	virtual void UpdateLayout() = 0;
};


} // namespace inl::gui
