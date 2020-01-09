#pragma once


#include "Control.hpp"


namespace inl::gui {


enum class eControlState {
	NORMAL,
	HOVERED,
	FOCUSED,
	HELD,
	DRAGGED,
};



class ControlStateTracker {
public:
	ControlStateTracker(Control* target);
	~ControlStateTracker();

	eControlState Get() const;

private:
	void OnMouseEnter(Control*);
	void OnMouseLeave(Control*);
	void OnGainFocus(Control*);
	void OnLoseFocus(Control*);
	void OnMouseDown(Control*, Vec2, eMouseButton);
	void OnMouseUp(Control*, Vec2, eMouseButton);
	void OnDragBegin(Control*, Vec2);
	void OnDragEnd(Control*, Vec2, Control*);

private:
	eControlState m_state = eControlState::NORMAL;
	Control* m_target;
	bool m_hovered = false;
	bool m_focused = false;
	bool m_held = false;
};



} // namespace inl::gui