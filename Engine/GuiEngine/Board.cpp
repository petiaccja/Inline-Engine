#include "Board.hpp"

#include "Layout.hpp"

#include <iostream>


namespace inl::gui {


template <class EventT, class... Args>
void Board::PropagateEventUpwards(Control* control, EventT event, Args&&... args) {
	while (control != nullptr) {
		(control->*event)(std::forward<Args>(args)...);
		control = control->GetParent();
	}
}


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

	if (target) {
		switch (evt.state) {
			case eKeyState::DOWN:
				if (evt.button == eMouseButton::LEFT) {
					m_dragPointOrigin = point;
					m_dragControlOrigin = target->GetPosition();
					m_draggedControl = target;
				}
				PropagateEventUpwards(target, &Control::OnMouseDown, target, point, evt.button);
				break;
			case eKeyState::UP:
				if (!m_focusedControl) {
					m_focusedControl = target;
					PropagateEventUpwards(target, &Control::OnGainFocus, target);
				}

				PropagateEventUpwards(target, &Control::OnMouseUp, target, point, evt.button);
				PropagateEventUpwards(target, &Control::OnClick, target, point, evt.button);
				break;
			case eKeyState::DOUBLE:
				PropagateEventUpwards(target, &Control::OnMouseDown, target, point, evt.button);
				PropagateEventUpwards(target, &Control::OnDoubleClick, target, point, evt.button);
				break;
		}
	}

	switch (evt.state) {
		case eKeyState::DOWN:
			if (m_focusedControl && target != m_focusedControl) {
				PropagateEventUpwards(m_focusedControl, &Control::OnLoseFocus, m_focusedControl);
				m_focusedControl = nullptr;
			}
			break;
		case eKeyState::UP:
			if (evt.button == eMouseButton::LEFT) {
				if (m_firstDrag != true && m_draggedControl) {
					PropagateEventUpwards(m_draggedControl, &Control::OnDragEnd, m_draggedControl, point, target);
				}
				m_draggedControl = nullptr;
				m_firstDrag = true;
			}
			break;
	}
}


void Board::OnMouseMove(MouseMoveEvent evt) {
	// Find Control under the mouse pointer.
	Vec2 point = { evt.absx, evt.absy };
	point = point * m_coordinateMapping;

	Control* target = const_cast<Control*>(GetTarget(point));

	// Handle leave area events.
	if (!target) {
		if (m_hoveredControl) {
			PropagateEventUpwards(m_hoveredControl, &Control::OnLeaveArea, m_hoveredControl);
			m_hoveredControl = nullptr;
		}
	}

	// Handle area enter and leave events.
	if (m_hoveredControl && m_hoveredControl != target) {
		PropagateEventUpwards(m_hoveredControl, &Control::OnLeaveArea, m_hoveredControl);
		m_hoveredControl = nullptr;
	}
	if (!m_hoveredControl) {
		m_hoveredControl = target;
		if (m_hoveredControl) {
			PropagateEventUpwards(target, &Control::OnEnterArea, target);
		}
	}

	// Hovering events.
	PropagateEventUpwards(target, &Control::OnHover, target, point);

	// Drag events.
	if (m_draggedControl) {
		if (m_firstDrag) {
			PropagateEventUpwards(m_draggedControl, &Control::OnDragBegin, m_draggedControl, point);
			m_firstDrag = false;
		}
		PropagateEventUpwards(m_draggedControl, &Control::OnDrag, m_draggedControl, point);
	}
}


void Board::OnKeyboard(KeyboardEvent evt) {
	if (m_focusedControl) {
		if (evt.state == eKeyState::DOWN) {
			PropagateEventUpwards(m_focusedControl, &Control::OnKeydown, m_focusedControl, evt.key);
		}
		if (evt.state == eKeyState::UP) {
			PropagateEventUpwards(m_focusedControl, &Control::OnKeyup, m_focusedControl, evt.key);
		}
	}

	// Debugging commands with F keys.
	if (evt.key == eKey::F1 && evt.state == eKeyState::DOWN) {
		DebugTree();
	}
	if (evt.key == eKey::F2 && evt.state == eKeyState::DOWN) {
		m_breakOnTrace = true;
	}
}


void Board::OnCharacter(char32_t evt) {
	if (m_focusedControl) {
		PropagateEventUpwards(m_focusedControl, &Control::OnCharacter, m_focusedControl, evt);
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


float Board::SetDepth(float depth) {
	m_depth = depth;
	return m_depthSpan;
}


float Board::GetDepth() const {
	return m_depth;
}


void Board::Update(float elapsed) {
	for (auto& child : m_controls) {
		UpdateLayouts(child.get());
		child->SetDepth(m_depth); // TODO: implement order by focus
		Update(child.get(), elapsed);
	}
}


void Board::Update(Control* subject, float elapsed) {
	if (subject == nullptr) {
		return;
	}

	subject->Update();

	auto children = subject->GetChildren();
	for (auto& child : children) {
		Update(const_cast<Control*>(child), elapsed);
	}
}


void Board::UpdateLayouts(Control* subject) {
	if (subject == nullptr) {
		return;
	}

	if (auto* layout = dynamic_cast<Layout*>(subject)) {
		layout->UpdateLayout();
	}

	auto children = subject->GetChildren();
	for (auto& child : children) {
		UpdateLayouts(const_cast<Control*>(child));
	}
}


const Control* Board::HitTestRecurse(Vec2 point, const Control* top) {
	if (HitTest(point, top)) {
		auto children = top->GetChildren();
		float maxDepth = -1e4f;
		const Control* finalHit = top;
		for (auto child : children) {
			const Control* childHit = HitTestRecurse(point, child);
			if (childHit && childHit->GetDepth() > maxDepth) {
				finalHit = childHit;
				maxDepth = childHit->GetDepth();
			}
		}
		return finalHit;
	}
	else {
		return nullptr;
	}
}


bool Board::HitTest(Vec2 point, const Control* control) {
	Vec2 pos = control->GetPosition();
	Vec2 size = control->GetSize();
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
	std::cout << "- " << typeid(*control).name() << " " << control->GetSize() << "\n";

	auto children = control->GetChildren();
	for (auto child : children) {
		DebugTreeRecurse(child, level + 1);
	}
}


const Control* Board::GetTarget(Vec2 point) const {
	const Control* target = nullptr;
	float topmostDepth = -1e4f;

#ifdef _WIN32
	if (m_breakOnTrace && IsDebuggerPresent()) {
		__debugbreak();
	}
#endif

	// TODO: handle depth
	for (auto child : m_controls) {
		auto* hit = HitTestRecurse(point, child.get());
		if (hit) {
			target = hit;
		}
	}
	m_breakOnTrace = false;
	return target;
}


const DrawingContext* Board::GetContext() const {
	return m_context.engine && m_context.scene ? &m_context : nullptr;
}


} // namespace inl::gui
