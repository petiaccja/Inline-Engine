#include "Control.hpp"
#include <BaseLibrary/Rect.hpp>


namespace inl::gui {


void Control::AddChild(std::shared_ptr<Control> child) {
	assert(child->m_parent == nullptr);

	auto [it, isNew] = m_children.insert(child);
	if (!isNew) {
		throw InvalidArgumentException("Specified control already a child of *this.");
	}
	child->m_parent = this;
	ChildAddedHandler(*child);
	child->AttachedHandler(*this);
	CallEventUpstream(&Control::OnChildAdded, this, child.get());
}


void Control::RemoveChild(const Control* child) {
	assert(child->m_parent == this);

	auto it = m_children.find(child);
	if (it != m_children.end()) {
		CallEventUpstream(&Control::OnChildRemoved, this, it->get());
		(*it)->DetachedHandler();
		ChildRemovedHandler(**it);
		(*it)->m_parent = nullptr;
		m_children.erase(it);
	}
	else {
		throw InvalidArgumentException("Specified control not a child of *this.");
	}
}


void Control::ClearChildren() {
	for (const auto& child : m_children) {
		child->DetachedHandler();
		ChildRemovedHandler(*child);
		child->m_parent = nullptr;
	}
	m_children.clear();
}


const Control* Control::GetParent() const {
	return m_parent;
}


void Control::SetStyle(const ControlStyle& style, bool useDefault) {
	m_style = style;
	m_usingDefaultStyle = useDefault;
	UpdateStyle();
}


const ControlStyle& Control::GetStyle() const {
	return m_style;
}


void Control::SetUsingDefaultStyle(bool enabled) {
	m_usingDefaultStyle = enabled;
}


bool Control::GetUsingDefaultStyle() const {
	return m_usingDefaultStyle;
}


std::set<Control*> Control::GetChildren() const {
	std::set<Control*> children;
	for (const auto& child : m_children) {
		children.insert(child.get());
	}
	return children;
}


bool Control::HitTest(const Vec2& point) const {
	Vec2 pos = GetPosition();
	Vec2 size = GetSize();
	RectF rc{ pos - size / 2, pos + size / 2 };

	return rc.IsPointInside(point);
}


} // namespace inl::gui