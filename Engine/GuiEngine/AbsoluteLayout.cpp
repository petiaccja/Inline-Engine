#include "AbsoluteLayout.hpp"

#include <BaseLibrary/Exception/Exception.hpp>


namespace inl::gui {


//------------------------------------------------------------------------------
// Children manipulation
//------------------------------------------------------------------------------

AbsoluteLayout::Binding& AbsoluteLayout::operator[](const Control* child) {
	if (child->GetParent() != this) {
		throw InvalidArgumentException("Child does not belong to this control.");
	}

	return GetLayoutPosition<Binding>(*child);
}


const AbsoluteLayout::Binding& AbsoluteLayout::operator[](const Control* child) const {
	if (child->GetParent() != this) {
		throw InvalidArgumentException("Child does not belong to this control.");
	}

	return GetLayoutPosition<const Binding>(*child);
}


//------------------------------------------------------------------------------
// Sizing
//------------------------------------------------------------------------------

void AbsoluteLayout::SetSize(const Vec2& size) {
	m_size = size;
	m_dirty = true;
}


Vec2 AbsoluteLayout::GetSize() const {
	return m_size;
}


Vec2 AbsoluteLayout::GetPreferredSize() const {
	const auto& children = GetChildren();

	if (children.empty()) {
		return { 0, 0 };
	}

	Vec2 minBound = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
	Vec2 maxBound = { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() };

	for (const auto& child : children) {
		const auto& binding = GetLayoutPosition<Binding>(*child);

		Vec2 pos = binding.GetPosition();
		Vec2 size = child->GetSize();

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

void AbsoluteLayout::SetPosition(const Vec2& position) {
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
// Layout update
//------------------------------------------------------------------------------

void AbsoluteLayout::UpdateLayout() {
	const auto& children = GetChildren();

	for (auto& child : children) {
		auto& binding = GetLayoutPosition<Binding>(*child);

		if (m_dirty || binding.m_dirty) {
			const_cast<Control*>(child)->SetPosition(CalculateChildPosition(binding));
		}
		binding.m_dirty = false;
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

Vec2 AbsoluteLayout::CalculateChildPosition(const Binding& binding) const {
	Vec2 pos = binding.GetPosition();
	if (m_yDown) {
		pos.y = -pos.y;
	}
	Vec2 offset = GetSize() / 2.f;
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


void AbsoluteLayout::ChildAddedHandler(Control& child) {
	m_childrenOrder.push_back(&child);
	auto orderIt = --m_childrenOrder.end();
	SetLayoutPosition(child, Binding(&m_childrenOrder, orderIt));
}


void AbsoluteLayout::ChildRemovedHandler(Control& child) {
	Binding& binding = GetLayoutPosition<Binding>(child);
	m_childrenOrder.erase(binding.orderIter);
}


AbsoluteLayout::Binding& AbsoluteLayout::Binding::SetPosition(Vec2 position) {
	this->position = position;
	m_dirty = true;
	return *this;
}

Vec2 AbsoluteLayout::Binding::GetPosition() const {
	return position;
}

auto AbsoluteLayout::Binding::MoveForward() -> Binding& {
	assert(orderList);

	auto prevIt = orderIter;
	--prevIt;
	if (prevIt != orderList->end()) {
		orderList->splice(prevIt, *orderList, orderIter);
	}

	return *this;
}

auto AbsoluteLayout::Binding::MoveBackward() -> Binding& {
	assert(orderList);

	auto nextIt = orderIter;
	++nextIt;
	if (nextIt != orderList->end()) {
		orderList->splice(++nextIt, *orderList, orderIter);
	}

	return *this;
}

auto AbsoluteLayout::Binding::MoveToFront() -> Binding& {
	assert(orderList);

	const auto firstIt = orderList->begin();
	if (firstIt != orderList->end()) {
		orderList->splice(firstIt, *orderList, orderIter);
	}

	return *this;
}

auto AbsoluteLayout::Binding::MoveToBack() -> Binding& {
	assert(orderList);

	const auto endIt = orderList->end();
	if (endIt != orderList->end()) {
		orderList->splice(endIt, *orderList, orderIter);
	}

	return *this;
}

} // namespace inl::gui
