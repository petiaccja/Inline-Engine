#include "Board.hpp"

#include <iostream>


namespace inl::gui {


void Board::AddControl(std::shared_ptr<Control> control) {
	auto [it, newlyAdded] = m_controls.insert(control);
	if (!newlyAdded) {
		throw InvalidCallException("Control already added.");
	}
	Control::Attach(this, control.get());
}


void Board::RemoveControl(Control* control) {
	auto it = m_controls.find(control);
	if (it != m_controls.end()) {
		m_controls.erase(it);
		Control::Detach(control);
	}
	else {
		throw InvalidCallException("Control is not part of this Board.");
	}
}


void Board::SetDrawingContext(DrawingContext context) {
	m_context = context;
}


const DrawingContext& Board::GetDrawingContext() const {
	return m_context;
}


void Board::SetStyle(nullptr_t) {
	m_defaultStyle = {};
}


void Board::SetStyle(const ControlStyle& style, bool asDefault) {
	m_defaultStyle = style;
}


const ControlStyle& Board::GetStyle() const {
	return m_defaultStyle;
}


void Board::OnMouseButton(MouseButtonEvent evt) {
	Vec2 point = { evt.x, evt.y };
	point = point * m_coordinateMapping;

	Control* target = const_cast<Control*>(GetTarget(point));
	if (!target) {
		return;
	}

	switch (evt.state) {
		case eKeyState::DOWN:
			if (evt.button == eMouseButton::LEFT) { m_dragSource = point; }
			if (m_focusedControl && target != m_focusedControl) { m_focusedControl->OnLoseFocus(); m_focusedControl = nullptr; }
			target->OnMouseDown(point, evt.button);
			break;
		case eKeyState::UP:
			if (evt.button == eMouseButton::LEFT) { m_dragSource.reset(); }
			if (!m_focusedControl) { m_focusedControl = target; target->OnGainFocus(); }
			target->OnMouseUp(point, evt.button);
			target->OnClick(point, evt.button);
			break;
		case eKeyState::DOUBLE:
			target->OnMouseDown(point, evt.button);
			target->OnDoubleClick(point, evt.button);
			break;
	}
}


void Board::OnMouseMove(MouseMoveEvent evt) {
	// Find Control under the mouse pointer.
	Vec2 point = { evt.absx, evt.absy };
	point = point * m_coordinateMapping;

	Control* target = const_cast<Control*>(GetTarget(point));
	   
	if (!target) {
		if (m_hoveredControl) {
			m_hoveredControl->OnLeaveArea();
			m_hoveredControl = nullptr;
		}
		return;
	}

	// Handle area enter and leave events.
	if (m_hoveredControl && m_hoveredControl != target) {
		m_hoveredControl->OnLeaveArea();
		m_hoveredControl = nullptr;
	}
	if (!m_hoveredControl) {
		m_hoveredControl = target;
		m_hoveredControl->OnEnterArea();
	}

	// Hovering events.
	target->OnHover(point);

	// Drag events.
	if (m_dragSource) {
		target->OnDrag(m_dragSource.value(), point);
	}
}


void Board::OnKeyboard(KeyboardEvent evt) {
	if (m_focusedControl) {
		if (evt.state == eKeyState::DOWN) {
			m_focusedControl->OnKeydown(evt.key);
		}
		if (evt.state == eKeyState::UP) {
			m_focusedControl->OnKeyup(evt.key);
		}
	}
}


void Board::OnCharacter(char32_t evt) {
	if (m_focusedControl) {
		m_focusedControl->OnCharacter(evt);
	}
}


void Board::SetCoordinateMapping(RectF window, RectF gui) {
	Mat33 transform = Mat33::Identity();
	transform *= Mat33::Translation(-window.GetCenter());
	transform *= Mat33::Scale(1.f / window.GetSize());
	transform *= Mat33::Scale(gui.GetSize());
	transform *= Mat33::Translation(gui.GetCenter());

	m_coordinateMapping = transform;
}


void Board::Update(float elapsed) {
	for (auto& child : m_controls) {
		child->Update(elapsed);
	}
	UpdateZOrder();
}


const Control* Board::HitTestRecurse(Vec2 point, const Control* top) {
	if (HitTest(point, top)) {
		auto children = top->GetChildren();
		for (auto child : children) {
			if (auto finalHit = HitTestRecurse(point, child)) {
				return finalHit;
			}
		}
		return top;
	}
	else {
		return nullptr;
	}
}


bool Board::HitTest(Vec2 point, const Control* control) {
	Vec2i pos = control->GetPosition();
	Vec2u size = control->GetSize();
	RectF rc{ pos - size / 2, pos + size / 2 };

	return rc.IsPointInside(point);
}


void Board::DebugTree() const {
	for (auto& child : m_controls) {
		DebugTreeRecurse(child.get(), 0);
	}
	std::cout.flush();
}


void Board::DebugTreeRecurse(const Control* control, int level) const {
	for (int i = 0; i < level; ++i) {
		std::cout << "  ";
	}
	std::cout << "- " << typeid(*control).name() << "\n";

	auto children = control->GetChildren();
	for (auto child : children) {
		DebugTreeRecurse(child, level + 1);
	}
}

const Control* Board::GetTarget(Vec2 point) const {
	const Control* target = nullptr;
	float topmostDepth = -1e4f;

	for (auto child : m_controls) {
		auto* hit = HitTestRecurse(point, child.get());
		if (hit && 0.0f > topmostDepth) {
			target = hit;
		}
	}

	return target;
}


void Board::UpdateZOrder() {
	for (auto& child: m_controls) {
		UpdateZOrderRecurse(child.get(), 0);
	}
}

void Board::UpdateZOrderRecurse(Control* control, int rank) {
	control->SetZOrder(rank);
	auto children = control->GetChildren();
	for (auto child : children) {
		UpdateZOrderRecurse(const_cast<Control*>(child), rank + 1);
	}
}


const DrawingContext* Board::GetContext() const {
	return m_context.engine && m_context.scene ? &m_context : nullptr;
}


} // namespace inl::gui
