#include "LinearLayout.hpp"


namespace inl::gui {


LinearLayout::LinearLayout(eDirection direction) {
	m_direction = direction;
}


void LinearLayout::Insert(const_iterator where, Control& control, CellSize sizing) {
	return Insert(where, MakeBlankShared(control), sizing);
}


void LinearLayout::Insert(const_iterator where, std::shared_ptr<Control> control, CellSize sizing) {
	auto mutWhere = m_children.emplace(where, std::move(control), sizing);
	if (mutWhere->control) {
		Attach(this, mutWhere->control.get());
	}
	m_dirty = true;
}


void LinearLayout::Change(const_iterator which, Control& control, CellSize sizing) {
	return Change(which, MakeBlankShared(control), sizing);
}


void LinearLayout::Change(const_iterator which, std::shared_ptr<Control> control, CellSize sizing) {
	auto mutWhich = m_children.begin() + (which - m_children.cbegin());

	if (mutWhich->control) {
		Detach(mutWhich->control.get());
	}
	mutWhich->control = std::move(control);
	if (mutWhich->control) {
		Attach(this, mutWhich->control.get());
	}
	mutWhich->sizing = sizing;

	m_dirty = true;
}


void LinearLayout::Change(const_iterator which, CellSize sizing) {
	auto mutWhich = m_children.begin() + (which - m_children.cbegin());
	mutWhich->sizing = sizing;

	m_dirty = true;
}


void LinearLayout::PushBack(Control& control, CellSize sizing) {
	return PushBack(MakeBlankShared(control), sizing);
}


void LinearLayout::PushBack(std::shared_ptr<Control> control, CellSize sizing) {
	m_children.emplace_back(std::move(control), sizing);
	if (m_children.back().control) {
		Attach(this, m_children.back().control.get());
	}

	m_dirty = true;
}


void LinearLayout::Erase(const_iterator which) {
	if (which->control) {
		Detach(which->control.get());
	}
	m_children.erase(which);

	m_dirty = true;
}


void LinearLayout::Clear() {
	for (auto& cell : m_children) {
		if (cell.control) {
			Detach(cell.control.get());
		}
	}
	m_children.clear();

	m_dirty = true;
}


LinearLayout::CellSize& LinearLayout::operator[](size_t slot) {
	return m_children[slot].sizing;
}


const LinearLayout::CellSize& LinearLayout::operator[](size_t slot) const {
	return m_children[slot].sizing;
}



void LinearLayout::SetSize(Vec2 size) {
	m_size = size;

	m_dirty = true;
}


Vec2 LinearLayout::GetSize() const {
	return m_size;
}


LinearLayout::SizingMeasurement LinearLayout::CalcMeasures() const {
	SizingMeasurement measures;

	for (const auto& child : m_children) {
		Vec2 preferredSize = child.control ? child.control->GetPreferredSize() : Vec2(0, 0);
		Vec2 minSize = child.control ? child.control->GetMinimumSize() : Vec2(0, 0);


		Vec2 margin = {
			child.sizing.GetMargin().left + child.sizing.GetMargin().right,
			child.sizing.GetMargin().top + child.sizing.GetMargin().bottom
		};

		// Main direction in X, fixed in Y
		if (m_direction == VERTICAL) {
			preferredSize = preferredSize.yx;
			minSize = minSize.yx;
			margin = margin.yx;
		}

		measures.maxPreferredAux = std::max(measures.maxPreferredAux, preferredSize.y);
		measures.minSizeAux = std::max(measures.minSizeAux, minSize.y + margin.y);

		switch (child.sizing.GetType()) {
			case eCellType::ABSOLUTE:
				measures.sumAbsolute += std::max(minSize.x + margin.x, child.sizing.GetValue());
				measures.sumMinSizeAbs += minSize.x;
				break;
			case eCellType::WEIGHT:
				measures.sumRelative += child.sizing.GetValue();
				measures.maxPreferredPerRel = std::max(measures.maxPreferredPerRel, (preferredSize.x + margin.x) / child.sizing.GetValue());
				measures.sumMinSizeRel += minSize.x;
				break;
			case eCellType::AUTO:
				measures.sumAbsolute += preferredSize.x + margin.x;
				measures.sumMinSizeAbs += minSize.x;
				break;
		}

		measures.sumMargins += margin.x;
	}

	return measures;
}


void LinearLayout::PositionChild(const Cell& cell, Vec2 childSize, float primaryOffset, Vec2 budgetSize) {
	Vec2 primaryDir = { 1.0f, 0.0f };
	Vec2 auxDir = { 0.0, 1.0f };

	float primaryMargins[2];
	float auxMargins[2];

	const auto& margin = cell.sizing.GetMargin();

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

	cell.control->SetPosition(pos);
	cell.control->SetSize(size);
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


void LinearLayout::SetPosition(Vec2 position) {
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

	for (const auto& child : m_children) {
		Vec2 preferredSize = child.control ? child.control->GetPreferredSize() : Vec2(0, 0);
		Vec2 minSize = child.control ? child.control->GetMinimumSize() : Vec2(0, 0);
		Vec2 margin = {
			child.sizing.GetMargin().left + child.sizing.GetMargin().right,
			child.sizing.GetMargin().top + child.sizing.GetMargin().bottom
		};

		// Main direction in X, fixed in Y
		if (m_direction == VERTICAL) {
			preferredSize = preferredSize.yx;
			minSize = minSize.yx;
			margin = margin.yx;
		}

		Vec2 size;
		size.y = auxBudget - margin.y;
		switch (child.sizing.GetType()) {
			case eCellType::ABSOLUTE: {
				float wanted = std::max(minSize.x + margin.x, child.sizing.GetValue());
				float overMin = wanted - minSize.x;
				float extraBudget = std::min(overMin, absoluteExtraBudget);
				absoluteExtraBudget -= extraBudget;
				size.x = minSize.x + overMin - margin.x;
				break;
			}
			case eCellType::WEIGHT: {
				float wanted = std::max(minSize.x + margin.x, std::floor(child.sizing.GetValue() * unitsPerRel));
				float overMin = wanted - minSize.x;
				float extraBudget = std::min(overMin, relativeExtraBudget);
				relativeExtraBudget -= extraBudget;
				size.x = minSize.x + overMin - margin.x;
				break;
			}
			case eCellType::AUTO:
				float wanted = std::max(minSize.x + margin.x, preferredSize.x);
				float overMin = wanted - minSize.x;
				float extraBudget = std::min(overMin, absoluteExtraBudget);
				absoluteExtraBudget -= extraBudget;
				size.x = minSize.x + overMin - margin.x;
				break;
		}

		if (child.control) {
			PositionChild(child, size, primaryOffset, { budget, auxBudget });
		}

		primaryOffset += size.x + margin.x;
	}
}


std::vector<const Control*> LinearLayout::GetChildren() const {
	std::vector<const Control*> children;
	children.reserve(m_children.size());
	for (auto& child : m_children) {
		if (child.control) {
			children.push_back(child.control.get());
		}
	}
	return children;
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
	for (auto& child : m_children) {
		if (child.control) {
			maxSpan = std::max(maxSpan, child.control->SetDepth(depth + 1.0f));
		}
	}
	return maxSpan + 1.0f;
}


float LinearLayout::GetDepth() const {
	return m_depth;
}


void LinearLayout::OnAttach(Control* parent) {
	Layout::OnAttach(parent);
	for (auto& child : m_children) {
		if (child.control) {
			Attach(this, child.control.get());
		}
	}

	m_parent = parent;
}

void LinearLayout::OnDetach() {
	for (auto& child : m_children) {
		if (child.control) {
			Detach(child.control.get());
		}
	}
	Layout::OnDetach();

	m_parent = nullptr;
}


} // namespace inl::gui
