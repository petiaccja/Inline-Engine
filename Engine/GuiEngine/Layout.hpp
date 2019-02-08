#pragma once

#include "Control.hpp"

namespace inl::gui {


class Layout : public Control {
public:
	virtual ~Layout() = default;

	// Visibility.
	void SetVisible(bool visible) override;
	bool GetVisible() const override;
	bool IsShown() const override;
	
	// Layout update
	virtual void UpdateLayout() = 0;
};


} // namespace inl::gui
