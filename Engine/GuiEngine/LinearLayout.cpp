#include "LinearLayout.hpp"


namespace inl::gui {


void LinearLayout::Insert(const_iterator where, Control& control, CellSize sizing) {
	return Insert(where, MakeBlankShared(control), sizing);
}


void LinearLayout::Insert(const_iterator where, std::shared_ptr<Control> control, CellSize sizing) {
	auto mutWhere = m_children.emplace(where, std::move(control), sizing);
	if (mutWhere->control) {
		Attach(this, mutWhere->control.get());
	}
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
}


void LinearLayout::Change(const_iterator which, CellSize sizing) {
	auto mutWhich = m_children.begin() + (which - m_children.cbegin());
	mutWhich->sizing = sizing;
}


void LinearLayout::PushBack(Control& control, CellSize sizing) {
	return PushBack(MakeBlankShared(control), sizing);
}


void LinearLayout::PushBack(std::shared_ptr<Control> control, CellSize sizing) {
	m_children.emplace_back(std::move(control), sizing);
	if (m_children.back().control) {
		Attach(this, m_children.back().control.get());
	}
}


void LinearLayout::Erase(const_iterator which) {
	if (which->control) {
		Detach(which->control.get());
	}
	m_children.erase(which);
}


void LinearLayout::Clear() {
	for (auto& cell : m_children) {
		if (cell.control) {
			Detach(cell.control.get());
		}
	}
	m_children.clear();
}


void LinearLayout::SetSize(Vec2u size) {
	m_size = size;
}
Vec2u LinearLayout::GetSize() const {
	return m_size;
}
void LinearLayout::SetPosition(Vec2i position) {
	m_position = position;
}
Vec2i LinearLayout::GetPosition() const {
	return m_position;
}


void LinearLayout::Update(float elapsed) {
	float sumPercentage = 0.0f;
	unsigned sumAbsolute = 0;
	for (const auto& child : m_children) {
		switch (child.sizing.GetType()) {
			case eCellType::ABSOLUTE: sumAbsolute += (unsigned)std::max(0.0f, child.sizing.GetValue()); break;
			case eCellType::WEIGHT: sumPercentage += std::max(0.0f, child.sizing.GetValue()); break;
			case eCellType::AUTO: sumAbsolute += 80; break; // Use Control::GetPreferred size instead.
		}
	}

	unsigned totalLength = !m_vertical ? GetSize().x : GetSize().y;
	unsigned weightedLength = std::max(0, (int)totalLength - (int)sumAbsolute);

	int whereMoving = GetPosition().x - GetSize().x/2;
	int whereFix = GetPosition().y - GetSize().y/2;
	if (m_vertical) {
		std::swap(whereMoving, whereFix);
	}
	if (m_inverted) {
		whereMoving += totalLength;
	}

	for (const auto& child : m_children) {
		int moving = 0;
		int fix = !m_vertical ? GetSize().y : GetSize().x;
		switch (child.sizing.GetType()) {
			case eCellType::ABSOLUTE: moving = (int)std::max(0.0f, child.sizing.GetValue()); break;
			case eCellType::WEIGHT: moving = (int)std::max(0.0f, child.sizing.GetValue())/sumPercentage*weightedLength; break;
			case eCellType::AUTO: moving = 80; break;
		}

		int totalMarginMoving = child.sizing.GetMargin().left + child.sizing.GetMargin().right;
		int totalMarginFix = child.sizing.GetMargin().top + child.sizing.GetMargin().bottom;

		int controlMoving = moving - totalMarginMoving;
		controlMoving = std::max(0, controlMoving);
		if (m_inverted) {
			moving *= -1;
			controlMoving *= -1;
		}

		int controlFix = fix - totalMarginFix;

		if (child.control) {
			Vec2i controlPos;
			Vec2u controlSize;
			int fixLowMargin = !m_vertical ? child.sizing.GetMargin().bottom : child.sizing.GetMargin().left;
			if (!m_vertical) {
				controlPos = { whereMoving + controlMoving / 2, whereFix + fixLowMargin + controlFix / 2 };
				controlSize = { std::abs(controlMoving), controlFix };
			}
			else {
				controlPos = { whereFix + fixLowMargin + controlFix / 2, whereMoving + controlMoving / 2 };
				controlSize = { controlFix, std::abs(controlMoving) };
			}
			child.control->SetPosition(controlPos);
			child.control->SetSize(controlSize);
		}
		
		whereMoving += moving;
	}

	for (const auto& child : m_children) {
		if (child.control) {
			child.control->Update(elapsed);
		}
	}

	SetDepth(m_depth);
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


float LinearLayout::SetDepth(float depth) {
	m_depth = depth;
	float maxSpan = 0.0f;
	for (auto& child : m_children) {
		maxSpan = std::max(maxSpan, child.control->SetDepth(depth + 1.0f));
	}
	return maxSpan + 1.0f;
}


float LinearLayout::GetDepth() const {
	return m_depth;
}


void LinearLayout::OnAttach(Control* parent) {
	Layout::OnAttach(parent);
	for (auto& child : m_children) {
		Attach(this, child.control.get());
	}

	m_parent = parent;
}

void LinearLayout::OnDetach() {
	for (auto& child : m_children) {
		Detach(child.control.get());
	}
	Layout::OnDetach();

	m_parent = nullptr;
}


} // namespace inl::gui
