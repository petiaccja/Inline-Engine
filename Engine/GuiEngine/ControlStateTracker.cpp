#include "ControlStateTracker.hpp"


namespace inl::gui {


ControlStateTracker::ControlStateTracker(Control* target) : m_target{ target } {
	target->OnEnterArea += Delegate<void(Control*)>{ &ControlStateTracker::OnMouseEnter, this };
	target->OnLeaveArea += Delegate<void(Control*)>{ &ControlStateTracker::OnMouseLeave, this };
	target->OnGainFocus += Delegate<void(Control*)>{ &ControlStateTracker::OnGainFocus, this };
	target->OnLoseFocus += Delegate<void(Control*)>{ &ControlStateTracker::OnLoseFocus, this };
	target->OnMouseDown += Delegate<void(Control*, Vec2, eMouseButton)>{ &ControlStateTracker::OnMouseDown, this };
	target->OnMouseUp += Delegate<void(Control*, Vec2, eMouseButton)>{ &ControlStateTracker::OnMouseUp, this };
	target->OnDragBegin += Delegate<void(Control*, Vec2)>{ &ControlStateTracker::OnDragBegin, this };
	target->OnDragEnd += Delegate<void(Control*, Vec2, Control*)>{ &ControlStateTracker::OnDragEnd, this };
}

ControlStateTracker::~ControlStateTracker() {
	m_target->OnEnterArea -= Delegate<void(Control*)>{ &ControlStateTracker::OnMouseEnter, this };
	m_target->OnLeaveArea -= Delegate<void(Control*)>{ &ControlStateTracker::OnMouseLeave, this };
	m_target->OnGainFocus -= Delegate<void(Control*)>{ &ControlStateTracker::OnGainFocus, this };
	m_target->OnLoseFocus -= Delegate<void(Control*)>{ &ControlStateTracker::OnLoseFocus, this };
	m_target->OnMouseDown -= Delegate<void(Control*, Vec2, eMouseButton)>{ &ControlStateTracker::OnMouseDown, this };
	m_target->OnMouseUp -= Delegate<void(Control*, Vec2, eMouseButton)>{ &ControlStateTracker::OnMouseUp, this };
	m_target->OnDragBegin -= Delegate<void(Control*, Vec2)>{ &ControlStateTracker::OnDragBegin, this };
	m_target->OnDragEnd -= Delegate<void(Control*, Vec2, Control*)>{ &ControlStateTracker::OnDragEnd, this };
}


void ControlStateTracker::OnMouseEnter(Control* arg) {
	if (arg != m_target) {
		return;
	}

	m_hovered = true;
	switch (m_state) {
		case eControlState::NORMAL:
			m_state = eControlState::HOVERED;
			break;
		default:
			return;
	}
}


void ControlStateTracker::OnMouseLeave(Control* arg) {
	if (arg != m_target) {
		return;
	}

	m_hovered = false;
	switch (m_state) {
		case eControlState::NORMAL: //[[fallthrough]]
		case eControlState::HOVERED:
			m_state = eControlState::NORMAL;
			break;
		default:
			return;
	}
}


void ControlStateTracker::OnGainFocus(Control* arg) {
	if (arg != m_target) {
		return;
	}

	m_focused = true;
	switch (m_state) {
		case eControlState::NORMAL:
		case eControlState::HOVERED:
			m_state = eControlState::FOCUSED;
			break;
		default:
			return;
	}
}


void ControlStateTracker::OnLoseFocus(Control* arg) {
	if (arg != m_target) {
		return;
	}

	m_focused = false;
	switch (m_state) {
		case eControlState::NORMAL:
		case eControlState::HOVERED:
		case eControlState::FOCUSED:
			m_state = m_hovered ? eControlState::HOVERED : eControlState::NORMAL;
			break;
		default:
			return;
	}
}


void ControlStateTracker::OnMouseDown(Control* arg, Vec2, eMouseButton) {
	if (arg != m_target) {
		return;
	}

	m_held = true;
	switch (m_state) {
		case eControlState::NORMAL:
		case eControlState::HOVERED:
		case eControlState::FOCUSED:
			m_state = eControlState::HELD;
			break;
		default:
			return;
	}
}


void ControlStateTracker::OnMouseUp(Control* arg, Vec2, eMouseButton) {
	if (arg != m_target) {
		return;
	}

	m_held = false;
	switch (m_state) {
		case eControlState::NORMAL:
		case eControlState::HOVERED:
		case eControlState::FOCUSED:
		case eControlState::HELD:
			m_state = m_focused ? eControlState::FOCUSED : (m_hovered ? eControlState::HOVERED : eControlState::NORMAL);
			break;
		default:
			return;
	}
}


void ControlStateTracker::OnDragBegin(Control* arg, Vec2) {
	if (arg != m_target) {
		return;
	}

	switch (m_state) {
		case eControlState::NORMAL:
		case eControlState::HOVERED:
		case eControlState::FOCUSED:
		case eControlState::HELD:
			m_state = eControlState::DRAGGED;
			break;
		default:
			return;
	}
}


void ControlStateTracker::OnDragEnd(Control* arg, Vec2, Control*) {
	if (arg != m_target) {
		return;
	}

	m_state =
		m_held ? eControlState::HELD : (m_focused ? eControlState::FOCUSED : (m_hovered ? eControlState::HOVERED : eControlState::NORMAL));
}


eControlState ControlStateTracker::Get() const {
	return m_state;
}


} // namespace inl::gui
