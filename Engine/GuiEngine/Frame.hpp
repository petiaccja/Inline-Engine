#pragma once


#include "Control.hpp"
#include "Layout.hpp"


namespace inl::gui {


class Frame : Control {
public:
	void SetLayout(std::shared_ptr<Layout> layout);
	const std::shared_ptr<Layout> GetLayout() const;
};


} // namespace inl::gui