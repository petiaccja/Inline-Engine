#pragma once

#include "Control.hpp"
#include <InlineMath.hpp>

#include <optional>


namespace inl::gui {


class Layout : public Control {
public:
	virtual ~Layout() = default;

	void SetVisible(bool visible) override;
	bool GetVisible() const override;
	bool IsShown() const override;

	void SetStyle(nullptr_t) override final;
	void SetStyle(const ControlStyle& style, bool asDefault = false) override final;
	const ControlStyle& GetStyle() const override final;

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



} // inl::gui