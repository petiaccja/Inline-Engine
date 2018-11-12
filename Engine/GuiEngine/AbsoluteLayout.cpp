#include "AbsoluteLayout.hpp"

#include <BaseLibrary/Exception/Exception.hpp>


namespace inl::gui {


AbsoluteLayout::Binding& AbsoluteLayout::AddChild(std::shared_ptr<Control> child) {
	auto [it, isNew] = m_children.insert({ child, std::make_unique<Binding>() });
	try {
		m_childrenOrder.push_front(child.get());
	} catch (...) {
		m_children.erase(it);
	}

	if (isNew) {
		Attach(this, child.get());
		it->second->orderList = &m_childrenOrder;
		it->second->orderIter = m_childrenOrder.begin();
		return *it->second;
	}
	else {
		throw InvalidArgumentException("Child already in layout.");
	}
}


void AbsoluteLayout::RemoveChild(Control* child) {
	auto it = m_children.find(child);

	if (it != m_children.end()) {
		Detach(it->first.get());
		m_childrenOrder.erase(it->second->orderIter);
		m_children.erase(it);
	}
	else {
		throw InvalidArgumentException("Child cannot be found.");
	}
}


void AbsoluteLayout::Clear() {
	for (auto& child : m_children) {
		Detach(child.first.get());
	}
	m_childrenOrder.clear();
	m_children.clear();
}


AbsoluteLayout::Binding& AbsoluteLayout::operator[](const Control* child) {
	auto it = m_children.find(child);
	if (it != m_children.end()) {
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


void AbsoluteLayout::Update(float elapsed) {
	for (auto& childBinding : m_children) {
		auto& [child, binding] = childBinding;
		child->SetPosition(CalculateChildPosition(*binding));
		child->Update(elapsed);
	}
	SetDepth(m_depth);
}


std::vector<const Control*> AbsoluteLayout::GetChildren() const {
	std::vector<const Control*> children;
	children.reserve(m_children.size());
	for (const auto& child : m_children) {
		children.push_back(child.first.get());
	}
	return children;
}


void AbsoluteLayout::SetReferencePoint(eRefPoint point) {
	m_refPoint = point;
}
AbsoluteLayout::eRefPoint AbsoluteLayout::GetReferencePoint() const {
	return m_refPoint;
}
void AbsoluteLayout::SetYDown(bool enabled) {
	m_yDown = enabled;
}
bool AbsoluteLayout::GetYDown() const {
	return m_yDown;
}


float AbsoluteLayout::SetDepth(float depth) {
	m_depth = depth;
	float totalSpan = 0.0f;
	for (auto it = m_childrenOrder.rbegin(), end = m_childrenOrder.rend(); it != end; ++it) {
		auto child = *it;
		totalSpan += child->SetDepth(depth + 1.0f + totalSpan);
	}
	return totalSpan + 1.0f;
}


float AbsoluteLayout::GetDepth() const {
	return m_depth;
}


void AbsoluteLayout::OnAttach(Control* parent) {
	Layout::OnAttach(parent);
	for (auto& child : m_children) {
		Attach(this, child.first.get());
	}

	m_parent = parent;
}

void AbsoluteLayout::OnDetach() {
	for (auto& child : m_children) {
		Detach(child.first.get());
	}
	Layout::OnDetach();

	m_parent = nullptr;
}


Vec2i AbsoluteLayout::CalculateChildPosition(const Binding& binding) const {
	Vec2i pos = binding.GetPosition();
	if (m_yDown) {
		pos.y = -pos.y;
	}
	Vec2i offset = GetSize() / 2;
	switch (m_refPoint) {
		case eRefPoint::TOPLEFT: offset *= { -1, 1 }; break;
		case eRefPoint::BOTTOMLEFT: offset *= { -1, -1 }; break;
		case eRefPoint::TOPRIGHT: offset *= { 1, 1 }; break;
		case eRefPoint::BOTTOMRIGHT: offset *= { 1, -1 }; break;
		case eRefPoint::CENTER: offset *= { 0, 0 }; break;
		default:;
	}
	return pos + offset + GetPosition();
}


AbsoluteLayout::Binding& AbsoluteLayout::Binding::SetPosition(Vec2i position) {
	this->position = position;
	return *this;
}

Vec2i AbsoluteLayout::Binding::GetPosition() const {
	return position;
}

void AbsoluteLayout::Binding::MoveForward() {
	assert(orderList);

	auto prevIt = orderIter;
	--prevIt;
	if (prevIt != orderList->end()) {
		orderList->splice(prevIt, *orderList, orderIter);
	}
}

void AbsoluteLayout::Binding::MoveBackward() {
	assert(orderList);

	auto nextIt = orderIter;
	++nextIt;
	if (nextIt != orderList->end()) {
		orderList->splice(++nextIt, *orderList, orderIter);
	}
}

void AbsoluteLayout::Binding::MoveToFront() {
	assert(orderList);

	const auto firstIt = orderList->begin();
	if (firstIt != orderList->end()) {
		orderList->splice(firstIt, *orderList, orderIter);
	}
}

void AbsoluteLayout::Binding::MoveToBack() {
	assert(orderList);

	const auto endIt = orderList->end();
	if (endIt != orderList->end()) {
		orderList->splice(endIt, *orderList, orderIter);
	}
}

} // namespace inl::gui
