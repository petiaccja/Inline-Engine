#include "Board.hpp"

#include "GraphicalControl.hpp"
#include "Layout.hpp"

#include <iostream>


namespace inl::gui {


Board::Board() {
	OnChildAdded += [this](Control*, Control* child) { SetGraphicsContext(child); };
	OnChildRemoved += [this](Control*, Control* child) { ClearGraphicsContext(child); };
}


void Board::SetDrawingContext(GraphicsContext context) {
	m_context = context;
	SetGraphicsContext(this);
}


const GraphicsContext& Board::GetDrawingContext() const {
	return m_context;
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
				target->CallEventUpstream(&Control::OnMouseDown, target, point, evt.button);
				break;
			case eKeyState::UP:
				if (!m_focusedControl) {
					m_focusedControl = target;
					target->CallEventUpstream(&Control::OnGainFocus, target);
				}

				target->CallEventUpstream(&Control::OnMouseUp, target, point, evt.button);
				target->CallEventUpstream(&Control::OnClick, target, point, evt.button);
				break;
			case eKeyState::DOUBLE:
				target->CallEventUpstream(&Control::OnMouseDown, target, point, evt.button);
				target->CallEventUpstream(&Control::OnDoubleClick, target, point, evt.button);
				break;
		}
	}

	switch (evt.state) {
		case eKeyState::DOWN:
			if (m_focusedControl && target != m_focusedControl) {
				m_focusedControl->CallEventUpstream(&Control::OnLoseFocus, m_focusedControl);
				m_focusedControl = nullptr;
			}
			break;
		case eKeyState::UP:
			if (evt.button == eMouseButton::LEFT) {
				if (m_firstDrag != true && m_draggedControl) {
					m_draggedControl->CallEventUpstream(&Control::OnDragEnd, m_draggedControl, point, target);
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
			m_hoveredControl->CallEventUpstream(&Control::OnLeaveArea, m_hoveredControl);
			m_hoveredControl = nullptr;
		}
	}

	// Handle area enter and leave events.
	if (m_hoveredControl && m_hoveredControl != target) {
		m_hoveredControl->CallEventUpstream(&Control::OnLeaveArea, m_hoveredControl);
		m_hoveredControl = nullptr;
	}
	if (!m_hoveredControl) {
		m_hoveredControl = target;
		if (m_hoveredControl) {
			target->CallEventUpstream(&Control::OnEnterArea, target);
		}
	}

	// Hovering events.
	target->CallEventUpstream(&Control::OnHover, target, point);

	// Drag events.
	if (m_draggedControl) {
		if (m_firstDrag) {
			m_draggedControl->CallEventUpstream(&Control::OnDragBegin, m_draggedControl, point);
			m_firstDrag = false;
		}
		m_draggedControl->CallEventUpstream(&Control::OnDrag, m_draggedControl, point);
	}
}


void Board::OnKeyboard(KeyboardEvent evt) {
	if (m_focusedControl) {
		if (evt.state == eKeyState::DOWN) {
			m_focusedControl->CallEventUpstream(&Control::OnKeydown, m_focusedControl, evt.key);
		}
		if (evt.state == eKeyState::UP) {
			m_focusedControl->CallEventUpstream(&Control::OnKeyup, m_focusedControl, evt.key);
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
		m_focusedControl->CallEventUpstream(&Control::OnCharacter, m_focusedControl, evt);
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
	const auto& children = GetChildren();
	for (auto& child : children) {
		UpdateLayouts(child);
		child->SetDepth(m_depth); // TODO: implement order by focus
		Update(child, elapsed);
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
	if (top->HitTest(point)) {
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

void Board::DebugTree() const {
	const auto& children = GetChildren();
	for (auto& child : children) {
		DebugTreeRecurse(child, 0);
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
	const auto& children = GetChildren();
	for (auto child : children) {
		auto* hit = HitTestRecurse(point, child);
		if (hit) {
			target = hit;
		}
	}
	m_breakOnTrace = false;
	return target;
}

void Board::SetGraphicsContext(Control* control) {
	ApplyRecurse(this, [this](Control* control) {
		if (GraphicalControl* graphical = dynamic_cast<GraphicalControl*>(control)) {
			graphical->SetContext(m_context);
		}
	});
}

void Board::ClearGraphicsContext(Control* control) {
	ApplyRecurse(this, [this](Control* control) {
		if (GraphicalControl* graphical = dynamic_cast<GraphicalControl*>(control)) {
			graphical->ClearContext();
		}
	});
}


} // namespace inl::gui
