#include "WindowLayout.hpp"

#include "BaseLibrary/Rect.hpp"


using inl::Vec2;



//------------------------------------------------------------------------------
// Children manipulation
//------------------------------------------------------------------------------


WindowLayout::Binding& WindowLayout::operator[](const Control* child) {
	if (child->GetParent() != this) {
		throw inl::InvalidArgumentException("Child does not belong to this control.");
	}

	return GetLayoutPosition<Binding>(*child);
}


const WindowLayout::Binding& WindowLayout::operator[](const Control* child) const {
	if (child->GetParent() != this) {
		throw inl::InvalidArgumentException("Child does not belong to this control.");
	}

	return GetLayoutPosition<const Binding>(*child);
}


//------------------------------------------------------------------------------
// Sizing
//------------------------------------------------------------------------------

void WindowLayout::SetSize(const Vec2& size) {
	m_size = size;
	m_dirty = true;
}


Vec2 WindowLayout::GetSize() const {
	return m_size;
}


Vec2 WindowLayout::GetPreferredSize() const {
	Vec2 largestChildPreferred = { 0, 0 };
	for (const auto& child : GetChildren()) {
		Vec2 childPreferred = child->GetPreferredSize();
		largestChildPreferred = Max(childPreferred, largestChildPreferred);
	}
	return largestChildPreferred;
}


Vec2 WindowLayout::GetMinimumSize() const {
	Vec2 largestChildMinimum = { 0, 0 };
	for (const auto& child : GetChildren()) {
		Vec2 childMinimum = child->GetMinimumSize();
		largestChildMinimum = Max(childMinimum, largestChildMinimum);
	}
	return largestChildMinimum;
}


//------------------------------------------------------------------------------
// Position & depth
//------------------------------------------------------------------------------

void WindowLayout::SetPosition(const Vec2& position) {
	m_position = position;
	m_dirty = true;
}


Vec2 WindowLayout::GetPosition() const {
	return m_position;
}


float WindowLayout::SetDepth(float depth) {
	m_depth = depth;
	float totalSpan = 0.0f;
	for (auto it = m_childrenOrder.rbegin(), end = m_childrenOrder.rend(); it != end; ++it) {
		auto child = *it;
		totalSpan += child->SetDepth(depth + 1.0f + totalSpan);
	}
	return totalSpan + 1.0f;
}


float WindowLayout::GetDepth() const {
	return m_depth;
}


//------------------------------------------------------------------------------
// Layout update
//------------------------------------------------------------------------------

void WindowLayout::UpdateLayout() {
	for (auto& child : GetChildren()) {
		auto& binding = GetLayoutPosition<Binding>(*child);

		if (m_dirty || binding.m_dirty) {
			PositionChild(*child, binding);
		}
		binding.m_dirty = false;
	}
	SetDepth(m_depth);

	m_dirty = false;
}


void WindowLayout::PositionChild(Control& child, Binding& binding) const {
	inl::RectF myBounds = inl::RectF::FromCenter({ 0, 0 }, GetSize());
	const auto& anchors = binding.GetAnchors();

	Vec2 pos, size;

	if (anchors.left && anchors.right) {
		pos.x = (myBounds.left + myBounds.right) / 2;
		size.x = binding.m_resizeToEdges ? myBounds.right - myBounds.left : child.GetSize().x;
	}
	else if (anchors.left) {
		pos.x = myBounds.left + child.GetSize().x / 2;
		size.x = child.GetSize().x;
	}
	else if (anchors.right) {
		pos.x = myBounds.right - child.GetSize().x / 2;
		size.x = child.GetSize().x;
	}
	else {
		pos.x = binding.GetPosition().x;
		size.x = child.GetSize().x;
	}


	if (anchors.bottom && anchors.top) {
		pos.y = (myBounds.bottom + myBounds.top) / 2;
		size.y = binding.m_resizeToEdges ? myBounds.top - myBounds.bottom : child.GetSize().y;
	}
	else if (anchors.bottom) {
		pos.y = myBounds.bottom + child.GetSize().y / 2;
		size.y = child.GetSize().y;
	}
	else if (anchors.top) {
		pos.y = myBounds.top - child.GetSize().y / 2;
		size.y = child.GetSize().y;
	}
	else {
		pos.y = binding.GetPosition().y;
		size.y = child.GetSize().y;
	}

	pos += GetPosition();

	child.SetPosition(pos);
	child.SetSize(size);
}


void WindowLayout::ChildAddedHandler(Control& child) {
	m_childrenOrder.push_back(&child);
	auto orderIt = --m_childrenOrder.end();
	SetLayoutPosition(child, Binding(&m_childrenOrder, orderIt));
}


void WindowLayout::ChildRemovedHandler(Control& child) {
	Binding& binding = GetLayoutPosition<Binding>(child);
	m_childrenOrder.erase(binding.orderIter);
}


//------------------------------------------------------------------------------
// Binding
//------------------------------------------------------------------------------

WindowLayout::Binding& WindowLayout::Binding::SetAnchors(bool left, bool right, bool bottom, bool top) {
	m_anchors = { left, right, bottom, top };
	return *this;
}


WindowLayout::Binding& WindowLayout::Binding::SetAnchors(Anchors anchors) {
	m_anchors = anchors;
	return *this;
}


WindowLayout::Anchors WindowLayout::Binding::GetAnchors() const {
	return m_anchors;
}


WindowLayout::Binding& WindowLayout::Binding::SetResizing(bool resizeToEdges) {
	m_resizeToEdges = resizeToEdges;
	return *this;
}


bool WindowLayout::Binding::GetResizing() const {
	return m_resizeToEdges;
}


WindowLayout::Binding& WindowLayout::Binding::MoveForward() {
	assert(orderList);

	auto prevIt = orderIter;
	--prevIt;
	if (prevIt != orderList->end()) {
		orderList->splice(prevIt, *orderList, orderIter);
	}

	return *this;
}


WindowLayout::Binding& WindowLayout::Binding::MoveBackward() {
	assert(orderList);

	auto nextIt = orderIter;
	++nextIt;
	if (nextIt != orderList->end()) {
		orderList->splice(++nextIt, *orderList, orderIter);
	}

	return *this;
}


WindowLayout::Binding& WindowLayout::Binding::MoveToFront() {
	assert(orderList);

	const auto firstIt = orderList->begin();
	if (firstIt != orderList->end()) {
		orderList->splice(firstIt, *orderList, orderIter);
	}

	return *this;
}


WindowLayout::Binding& WindowLayout::Binding::MoveToBack() {
	assert(orderList);

	const auto endIt = orderList->end();
	if (endIt != orderList->end()) {
		orderList->splice(endIt, *orderList, orderIter);
	}

	return *this;
}


WindowLayout::Binding& WindowLayout::Binding::SetPosition(Vec2 pos) {
	m_position = pos;
	m_dirty = true;
	return *this;
}

Vec2 WindowLayout::Binding::GetPosition() const {
	return m_position;
}
