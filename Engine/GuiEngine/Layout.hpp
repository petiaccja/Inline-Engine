#pragma once

#include "Control.hpp"
#include <InlineMath.hpp>

#include <optional>


namespace inl::gui {


class Layout : public Control {
public:
	virtual ~Layout() = default;

	virtual bool IsShown() const override { return true; }
};



} // inl::gui