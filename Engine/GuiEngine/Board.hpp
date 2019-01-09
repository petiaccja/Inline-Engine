#pragma once

#include "Control.hpp"

#include <BaseLibrary/Platform/Input.hpp>
#include <set>


namespace inl::gui {

//static_assert(false, "Cached objects, such as control dragged, focused, can be deleted while in cache causing a crash.")
// This can be solved by for example an OnDestroy callback on the Controls, or by using shared_ptrs as cache.


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

	float SetDepth(float depth) override;
	float GetDepth() const override;

	void Update(float elapsed) override;

private:
	void Update(Control* subject, float elapsed);
	void UpdateLayouts();
	void UpdateLayouts(Control* subject);

	static const Control* HitTestRecurse(Vec2 point, const Control* top);
	static bool HitTest(Vec2 point, const Control* control);
	const Control* GetTarget(Vec2 point) const;

	template <class EventT, class... Args>
	void PropagateEventUpwards(Control* control, EventT event, Args&&... args);

	void DebugTree() const;
	void DebugTreeRecurse(const Control* control, int level) const;

private:
	// Dummy implementations for Control.
	void SetSize(Vec2) override {}
	Vec2 GetSize() const override { return { 10000000, 10000000 }; }
	Vec2 GetMinimumSize() const override { return { 0,0 }; }
	Vec2 GetPreferredSize() const override { return { 0,0 }; }
	Control* GetParent() const override { return nullptr; }
	std::vector<const Control*> GetChildren() const override { return {}; }

	void SetPosition(Vec2) override {}
	Vec2 GetPosition() const override { return { 0,0 }; }

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
	Control* m_draggedControl = nullptr;
	Vec2 m_dragControlOrigin;
	Vec2 m_dragPointOrigin;
	bool m_firstDrag = true;

	Mat33 m_coordinateMapping = Mat33::Identity();
	mutable bool m_breakOnTrace = false;
	float m_depth = 0.0f;
	float m_depthSpan = -1.0f; // TODO: implement properly
};


} // namespace inl::gui
