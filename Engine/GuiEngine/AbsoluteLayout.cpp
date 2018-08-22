#include "AbsoluteLayout.hpp"
#include <BaseLibrary/Exception/Exception.hpp>


namespace inl::gui {


AbsoluteLayout::Binding& AbsoluteLayout::AddChild(std::shared_ptr<Control> child) {
	auto insIt = m_children.insert({ child, std::make_unique<Binding>() });
	if (insIt.second) {
		Attach(this, child.get());
		return *insIt.first->second;
	}
	else {
		throw InvalidArgumentException("Child already in layout.");
	}
}


void AbsoluteLayout::RemoveChild(const Control* child) {
	auto it = m_children.find(child);
	[[likely]]
	if (it != m_children.end()) {
		Detach(it->first.get());
		m_children.erase(it);
	}
	else {
		throw InvalidArgumentException("Child cannot be found.");
	}
}


AbsoluteLayout::Binding& AbsoluteLayout::operator[](const Control* child) {
	auto it = m_children.find(child);
	[[likely]]
	if (it != m_children.end()) {
		m_children.erase(it);
		return *it->second;
	}
	else {
		throw InvalidArgumentException("Child cannot be found.");
	}
}


void AbsoluteLayout::SetSize(Vec2u size) {
	m_size = size;
}


Vec2u AbsoluteLayout::GetSize() const {
	return m_size;
}


void AbsoluteLayout::SetPosition(Vec2i position) {
	m_position = position;
}


Vec2i AbsoluteLayout::GetPosition() const {
	return m_position;
}


void AbsoluteLayout::SetVisible(bool visible) {
	// empty
}


bool AbsoluteLayout::GetVisible() const {
	return false;
}


void AbsoluteLayout::Update(float elapsed) {
	for (auto& childBinding : m_children) {
		auto&[child, binding] = childBinding;
		child->SetPosition(GetPosition() + binding->GetPosition());
		child->Update(elapsed);
	}
}


} // inl::gui
