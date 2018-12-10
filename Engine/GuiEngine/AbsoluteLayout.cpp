#include "AbsoluteLayout.hpp"

#include <BaseLibrary/Exception/Exception.hpp>


namespace inl::gui {


//------------------------------------------------------------------------------
// Children manipulation
//------------------------------------------------------------------------------
AbsoluteLayout::Binding& AbsoluteLayout::AddChild(std::shared_ptr<Control> child) {
	auto [it, isNew] = m_children.insert({ child, std::make_unique<Binding>() });
	try {
		m_childrenOrder.push_front(child.get());
	}
	catch (...) {
		m_children.erase(it);
	}

	if (isNew) {
		Attach(this, child.get());
		it->second->orderList = &m_childrenOrder;
		it->second->orderIter = m_childrenOrder.begin();
		return *it->second;
	}
	throw InvalidArgumentException("Child already in layout.");
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
	throw InvalidArgumentException("Child cannot be found.");
}


//------------------------------------------------------------------------------
// Sizing
//------------------------------------------------------------------------------

void AbsoluteLayout::SetSize(Vec2 size) {
	m_size = size;

	m_dirty = true;
}


Vec2 AbsoluteLayout::GetSize() const {
	return m_size;
}


Vec2 AbsoluteLayout::GetPreferredSize() const {
	if (m_children.empty()) {
		return { 0, 0 };
	}

	Vec2 minBound = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
	Vec2 maxBound = { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() };

	for (auto& childBinding : m_children) {
		Vec2 pos = childBinding.first->GetPosition();
		Vec2 size = childBinding.first->GetSize();

		Vec2 minCorner = pos - size / 2.0f;
		Vec2 maxCorner = pos + size / 2.0f;

		minBound = Min(minBound, minCorner);
		maxBound = Min(maxBound, maxCorner);
	}

	return maxBound - minBound;
}

Vec2 AbsoluteLayout::GetMinimumSize() const {
	return { 0, 0 };
}


//------------------------------------------------------------------------------
// Position & depth
//------------------------------------------------------------------------------

void AbsoluteLayout::SetPosition(Vec2 position) {
	m_position = position;

	m_dirty = true;
}


Vec2 AbsoluteLayout::GetPosition() const {
	return m_position;
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


//------------------------------------------------------------------------------
// Hierarchy
//------------------------------------------------------------------------------


std::vector<const Control*> AbsoluteLayout::GetChildren() const {
	std::vector<const Control*> children;
	children.reserve(m_children.size());
	for (const auto& child : m_children) {
		children.push_back(child.first.get());
	}
	return children;
}


//------------------------------------------------------------------------------
// Layout update
//------------------------------------------------------------------------------

void AbsoluteLayout::UpdateLayout() {
	for (auto& childBinding : m_children) {
		if (m_dirty || childBinding.second->m_dirty) {
			auto&[child, binding] = childBinding;
			child->SetPosition(CalculateChildPosition(*binding));
		}
		childBinding.second->m_dirty = false;
	}
	SetDepth(m_depth);

	m_dirty = false;
}


//------------------------------------------------------------------------------
// Absolute layout
//------------------------------------------------------------------------------

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


//------------------------------------------------------------------------------
// Layout
//------------------------------------------------------------------------------

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


Vec2 AbsoluteLayout::CalculateChildPosition(const Binding& binding) const {
	Vec2 pos = binding.GetPosition();
	if (m_yDown) {
		pos.y = -pos.y;
	}
	Vec2 offset = GetSize() / 2;
	switch (m_refPoint) {
		case eRefPoint::TOPLEFT:
			offset *= { -1, 1 };
			break;
		case eRefPoint::BOTTOMLEFT:
			offset *= { -1, -1 };
			break;
		case eRefPoint::TOPRIGHT:
			offset *= { 1, 1 };
			break;
		case eRefPoint::BOTTOMRIGHT:
			offset *= { 1, -1 };
			break;
		case eRefPoint::CENTER:
			offset *= { 0, 0 };
			break;
		default:;
	}
	return pos + offset + GetPosition();
}


AbsoluteLayout::Binding& AbsoluteLayout::Binding::SetPosition(Vec2 position) {
	this->position = position;
	m_dirty = true;
	return *this;
}

Vec2 AbsoluteLayout::Binding::GetPosition() const {
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
