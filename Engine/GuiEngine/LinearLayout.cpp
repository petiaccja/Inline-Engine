#include "LinearLayout.hpp"


namespace inl::gui {


//------------------------------------------------------------------------------
// Binding
//------------------------------------------------------------------------------

LinearLayout::CellSize& LinearLayout::CellSize::SetWidth(float width) {
	type = eCellType::ABSOLUTE;
	value = width;
	return *this;
}

LinearLayout::CellSize& LinearLayout::CellSize::SetWeight(float weight) {
	type = eCellType::WEIGHT;
	value = std::max(0.0f, weight);
	return *this;
}

LinearLayout::CellSize& LinearLayout::CellSize::SetAuto() {
	type = eCellType::AUTO;
	return *this;
}

LinearLayout::CellSize& LinearLayout::CellSize::SetMargin(Rect<float, false, false> margin) {
	this->margin = margin;
	return *this;
}

LinearLayout::eCellType LinearLayout::CellSize::GetType() const {
	return type;
}

float LinearLayout::CellSize::GetValue() const {
	return value;
}

auto LinearLayout::CellSize::MoveForward() -> CellSize& {
	assert(orderList);

	auto prevIt = orderIter;
	--prevIt;
	if (prevIt != orderList->end()) {
		orderList->splice(prevIt, *orderList, orderIter);
	}

	return *this;
}

auto LinearLayout::CellSize::MoveBackward() -> CellSize& {
	assert(orderList);

	auto nextIt = orderIter;
	++nextIt;
	if (nextIt != orderList->end()) {
		orderList->splice(++nextIt, *orderList, orderIter);
	}

	return *this;
}

auto LinearLayout::CellSize::MoveToFront() -> CellSize& {
	assert(orderList);

	const auto firstIt = orderList->begin();
	if (firstIt != orderList->end()) {
		orderList->splice(firstIt, *orderList, orderIter);
	}

	return *this;
}

auto LinearLayout::CellSize::MoveToBack() -> CellSize& {
	assert(orderList);

	const auto endIt = orderList->end();
	if (endIt != orderList->end()) {
		orderList->splice(endIt, *orderList, orderIter);
	}

	return *this;
}

//------------------------------------------------------------------------------
// Layout
//------------------------------------------------------------------------------

LinearLayout::LinearLayout(eDirection direction) {
	m_direction = direction;
}


LinearLayout::CellSize& LinearLayout::operator[](const Control* child) {
	if (child->GetParent() != this) {
		throw InvalidArgumentException("Child does not belong to this control.");
	}
	return GetLayoutPosition<CellSize>(*child);
}


const LinearLayout::CellSize& LinearLayout::operator[](const Control* child) const {
	if (child->GetParent() != this) {
		throw InvalidArgumentException("Child does not belong to this control.");
	}
	return GetLayoutPosition<const CellSize>(*child);
}



void LinearLayout::SetSize(const Vec2& size) {
	m_size = size;
	m_dirty = true;
}


Vec2 LinearLayout::GetSize() const {
	return m_size;
}


LinearLayout::SizingMeasurement LinearLayout::CalcMeasures() const {
	SizingMeasurement measures;

	for (const auto& child : m_childrenOrder) {
		const auto& sizing = GetLayoutPosition<const CellSize>(*child);

		Vec2 preferredSize = child->GetPreferredSize();
		Vec2 minSize = child->GetMinimumSize();

		Vec2 margin = {
			sizing.GetMargin().left + sizing.GetMargin().right,
			sizing.GetMargin().top + sizing.GetMargin().bottom
		};

		// Main direction in X, fixed in Y
		if (m_direction == VERTICAL) {
			preferredSize = preferredSize.yx;
			minSize = minSize.yx;
			margin = margin.yx;
		}

		measures.maxPreferredAux = std::max(measures.maxPreferredAux, preferredSize.y + margin.y);
		measures.minSizeAux = std::max(measures.minSizeAux, minSize.y + margin.y);

		switch (sizing.GetType()) {
			case eCellType::ABSOLUTE:
				measures.sumAbsolute += std::max(minSize.x + margin.x, sizing.GetValue());
				measures.sumMinSizeAbs += minSize.x + margin.x;
				break;
			case eCellType::WEIGHT:
				measures.sumRelative += sizing.GetValue();
				measures.maxPreferredPerRel = std::max(measures.maxPreferredPerRel, (preferredSize.x + margin.x) / sizing.GetValue());
				measures.sumMinSizeRel += minSize.x + margin.x;
				break;
			case eCellType::AUTO:
				measures.sumAbsolute += preferredSize.x + margin.x;
				measures.sumMinSizeAbs += minSize.x + margin.x;
				break;
		}

		measures.sumMargins += margin.x;
	}

	return measures;
}


void LinearLayout::PositionChild(Control& child, Vec2 childSize, float primaryOffset, Vec2 budgetSize) {
	const CellSize& sizing = GetLayoutPosition<const CellSize>(child);

	Vec2 primaryDir = { 1.0f, 0.0f };
	Vec2 auxDir = { 0.0, 1.0f };

	float primaryMargins[2];
	float auxMargins[2];

	const auto& margin = sizing.GetMargin();

	auxMargins[0] = margin.bottom;
	auxMargins[1] = margin.top;
	primaryMargins[0] = margin.left;
	primaryMargins[1] = margin.right;

	if (m_direction == VERTICAL) {
		primaryDir = primaryDir.yx;
		auxDir = auxDir.yx;
		std::swap(primaryMargins[0], auxMargins[0]);
		std::swap(primaryMargins[1], auxMargins[1]);
	}

	if (m_inverted) {
		primaryDir *= -1.0f;
		std::swap(primaryMargins[0], primaryMargins[1]);
	}


	Vec2 base = GetPosition() - GetSize() / 2.0f * primaryDir - auxDir * budgetSize.y / 2.0f;

	float primaryPosOffset = primaryOffset + primaryMargins[0] + childSize.x / 2.0f;
	float auxPosOffset = auxMargins[0] + childSize.y / 2.0f;

	Vec2 pos = primaryPosOffset * primaryDir + auxPosOffset * auxDir + base;
	Vec2 size = m_direction == HORIZONTAL ? childSize : childSize.yx;

	child.SetPosition(pos);
	child.SetSize(size);
}


Vec2 LinearLayout::GetPreferredSize() const {
	SizingMeasurement measures = CalcMeasures();

	float unitsPerRel = measures.maxPreferredPerRel;

	float totalUnits = unitsPerRel * measures.sumRelative + measures.sumAbsolute;
	Vec2 preferredSize = { totalUnits, measures.maxPreferredAux };
	if (m_direction == VERTICAL) {
		preferredSize = preferredSize.yx;
	}

	return preferredSize;
}


Vec2 LinearLayout::GetMinimumSize() const {
	SizingMeasurement measures = CalcMeasures();

	Vec2 minSize = { measures.sumMinSizeAbs + measures.sumMinSizeAbs, measures.maxPreferredAux };
	if (m_direction == VERTICAL) {
		minSize = minSize.yx;
	}

	return minSize;
}


void LinearLayout::SetPosition(const Vec2& position) {
	m_position = position;

	m_dirty = true;
}


Vec2 LinearLayout::GetPosition() const {
	return m_position;
}


void LinearLayout::UpdateLayout() {
	if (!m_dirty) {
		return;
	}
	m_dirty = false;

	SizingMeasurement measures = CalcMeasures();

	float minSize = measures.sumMinSizeAbs + measures.sumMinSizeRel;
	float primarySize = m_direction == HORIZONTAL ? GetSize().x : GetSize().y;
	float auxSize = m_direction == HORIZONTAL ? GetSize().y : GetSize().x;
	float budget = std::max(minSize, primarySize);
	float auxBudget = std::max(auxSize, measures.minSizeAux);


	// First assign extra space over minSize to absolute children.
	float absoluteExtraPreferred = measures.sumAbsolute - measures.sumMinSizeAbs;
	float absoluteExtraAvailable = budget - minSize;
	float absoluteExtraBudget = std::min(absoluteExtraPreferred, absoluteExtraAvailable);

	// Assign remaining space to grow relative children over their minimal size.
	float relativeExtraBudget = budget - minSize - absoluteExtraBudget;
	float unitsPerRel = (measures.sumMinSizeRel + relativeExtraBudget) / measures.sumRelative;

	float primaryOffset = 0.0f;
	
	for (auto child : m_childrenOrder) {
		const CellSize& sizing = GetLayoutPosition<const CellSize>(*child);

		Vec2 preferredSize = child->GetPreferredSize();
		Vec2 minSize = child->GetMinimumSize();
		Vec2 margin = {
			sizing.GetMargin().left + sizing.GetMargin().right,
			sizing.GetMargin().top + sizing.GetMargin().bottom
		};

		// Main direction in X, fixed in Y
		if (m_direction == VERTICAL) {
			preferredSize = preferredSize.yx;
			minSize = minSize.yx;
			margin = margin.yx;
		}

		Vec2 size;
		size.y = auxBudget - margin.y;
		switch (sizing.GetType()) {
			case eCellType::ABSOLUTE: {
				float wanted = std::max(minSize.x + margin.x, sizing.GetValue());
				float overMin = wanted - minSize.x;
				float extraBudget = std::min(overMin, absoluteExtraBudget);
				absoluteExtraBudget -= extraBudget;
				size.x = minSize.x + overMin - margin.x;
				break;
			}
			case eCellType::WEIGHT: {
				float wanted = std::max(minSize.x + margin.x, std::floor(sizing.GetValue() * unitsPerRel));
				float overMin = wanted - minSize.x;
				float extraBudget = std::min(overMin, relativeExtraBudget);
				relativeExtraBudget -= extraBudget;
				size.x = minSize.x + overMin - margin.x;
				break;
			}
			case eCellType::AUTO:
				float wanted = std::max(minSize.x + margin.x, preferredSize.x + margin.x);
				float overMin = wanted - minSize.x;
				float extraBudget = std::min(overMin, absoluteExtraBudget);
				absoluteExtraBudget -= extraBudget;
				size.x = minSize.x + overMin - margin.x;
				break;
		}

		PositionChild(const_cast<Control&>(*child), size, primaryOffset, { budget, auxBudget });


		primaryOffset += size.x + margin.x;
	}
}



void LinearLayout::SetDirection(eDirection direction) {
	m_direction = direction;
}


LinearLayout::eDirection LinearLayout::GetDirection() {
	return m_direction;
}


float LinearLayout::SetDepth(float depth) {
	m_depth = depth;
	float maxSpan = 0.0f;

	const auto& children = GetChildren();

	for (auto& child : children) {
		maxSpan = std::max(maxSpan, const_cast<Control*>(child)->SetDepth(depth + 1.0f));
	}
	return maxSpan + 1.0f;
}


float LinearLayout::GetDepth() const {
	return m_depth;
}


void LinearLayout::ChildAddedHandler(Control& child) {
	m_childrenOrder.push_back(&child);
	auto orderIt = --m_childrenOrder.end();
	SetLayoutPosition(child, CellSize(&m_childrenOrder, orderIt));
}


void LinearLayout::ChildRemovedHandler(Control& child) {
	CellSize& binding = GetLayoutPosition<CellSize>(child);
	m_childrenOrder.erase(binding.orderIter);
}


} // namespace inl::gui
