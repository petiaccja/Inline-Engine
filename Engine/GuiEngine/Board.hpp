#pragma once

#include "Control.hpp"

#include <BaseLibrary/Platform/Input.hpp>
#include <set>


namespace inl::gui {


class Board : private Control {
public:
	void AddControl(Control& control) { AddControl(MakeBlankShared(control)); }
	void AddControl(std::shared_ptr<Control> control);
	void RemoveControl(Control* control);

	void SetDrawingContext(DrawingContext context);
	const DrawingContext& GetDrawingContext() const;

	void SetStyle(nullptr_t) override;
	void SetStyle(const ControlStyle& style, bool asDefault = false) override;
	const ControlStyle& GetStyle() const override;

	// Even handlers.
	void OnMouseButton(MouseButtonEvent evt);
	void OnMouseMove(MouseMoveEvent evt);
	void OnKeyboard(KeyboardEvent evt);
	void OnCharacter(char32_t evt);

	void SetCoordinateMapping(RectF window, RectF gui);

	void Update(float elapsed) override;

private:
	static const Control* HitTestRecurse(Vec2 point, const Control* top);
	static bool HitTest(Vec2 point, const Control* control);

	void DebugTree() const;
	void DebugTreeRecurse(const Control* control, int level) const;
	const Control* GetTarget(Vec2 point) const;

	void UpdateZOrder();
	static void UpdateZOrderRecurse(Control* control, int rank);
private:
	// Dummy implementations for Control.
	void SetSize(Vec2u) override {}
	Vec2u GetSize() const override { return { 0,0 }; }

	void SetPosition(Vec2i) override {}
	Vec2i GetPosition() const override { return { 0,0 }; }

	void SetVisible(bool) override {}
	bool GetVisible() const override { return true; }
	bool IsShown() const override { return true; }

	void OnAttach(Control* parent) override {}
	void OnDetach() override {}
	const DrawingContext* GetContext() const override;
private:
	DrawingContext m_context;
	ControlStyle m_defaultStyle;

	std::set<std::shared_ptr<Control>, impl::ControlPtrLess> m_controls;

	Control* m_focusedControl = nullptr;
	Control* m_hoveredControl = nullptr;
	std::optional<Vec2> m_dragSource;

	Mat33 m_coordinateMapping = Mat33::Identity();
	mutable bool m_breakOnTrace = false;
};



} // namespace inl::gui
