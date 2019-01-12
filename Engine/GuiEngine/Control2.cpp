#include "Control2.hpp"


namespace inl::gui {


void Control2::AddChild(std::shared_ptr<Control2> child) {
	assert(child->m_parent != nullptr);

	auto [it, isNew] = m_children.insert(child);
	if (!isNew) {
		throw InvalidArgumentException("Specified control already a child of *this.");
	}
	child->m_parent = this;
	ChildAddedHandler(*child);
	child->AttachedHandler(*this);
}


void Control2::RemoveChild(const Control2* child) {
	assert(child->m_parent == this);

	auto it = m_children.find(child);
	if (it != m_children.end()) {
		(*it)->DetachedHandler();
		ChildRemovedHandler(**it);
		(*it)->m_parent = nullptr;
		m_children.erase(it);
	}
	else {
		throw InvalidArgumentException("Specified control not a child of *this.");
	}
}


void Control2::ClearChildren() {
	for (const auto& child : m_children) {
		child->DetachedHandler();
		ChildRemovedHandler(*child);
		child->m_parent = nullptr;
	}
	m_children.clear();
}


const Control2* Control2::GetParent() const {
	return m_parent;
}


std::set<const Control2*> Control2::GetChildren() const {
	std::set<const Control2*> children;
	for (const auto& child : m_children) {
		children.insert(child.get());
	}
	return children;
}


} // namespace inl::gui