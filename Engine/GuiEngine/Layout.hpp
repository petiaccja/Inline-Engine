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

	// Style
	void SetStyle(nullptr_t) override final;
	void SetStyle(const ControlStyle& style, bool asDefault = false) override final;
	const ControlStyle& GetStyle() const override final;

	// Hierarchy
	Control* GetParent() const override final;

	// Layout update
	virtual void UpdateLayout() = 0;

protected:
	void OnAttach(Control* parent) override;
	void OnDetach() override;
	const DrawingContext* GetContext() const override final { return m_context; }

private:
	bool m_isStyleInherited = true;
	ControlStyle m_style;
	const DrawingContext* m_context = nullptr;
	Control* m_parent = nullptr;
};


} // namespace inl::gui
