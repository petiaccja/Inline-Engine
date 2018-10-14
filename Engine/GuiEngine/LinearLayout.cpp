#include "LinearLayout.hpp"


namespace inl::gui {


void LinearLayout::Cell::SetControl(std::shared_ptr<Control> control) {
	if (this->control) {
		Control::Detach(this->control.get());
	}
	this->control = control;
	if (this->control && parent) {
		Control::Attach(parent, this->control.get());
	}
}

LinearLayout::CellSize& LinearLayout::AddChild(Control& child, size_t index) {
	return AddChild(MakeBlankShared(child), index);
}


LinearLayout::CellSize& LinearLayout::AddChild(std::shared_ptr<Control> child, size_t index) {
	if (m_children.size() < index) {
		m_children.resize(index);
		m_children.push_back({ this, child, CellSize{} });
	}
	else {
		m_children.insert(m_children.begin() + index, { this, child, CellSize{} });
	}
	Attach(this, child.get());
	return m_children[index].size;
}


void LinearLayout::RemoveChild(size_t index) {
	assert(index < m_children.size());
	auto it = m_children.begin() + index;
	Detach(it->control.get());
	m_children.erase(it);
}

LinearLayout::Cell& LinearLayout::operator[](size_t index) {
	assert(index < m_children.size());
	return m_children[index];
}

const LinearLayout::Cell& LinearLayout::operator[](size_t index) const {
	assert(index < m_children.size());
	return m_children[index];
}

void LinearLayout::SetNumCells(size_t size) {
	m_children.resize(size);
	for (auto& child : m_children) {
		child.parent = this;
	}
}

size_t LinearLayout::GetNumCells() const {
	return m_children.size();
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
		switch (child.size.type) {
			case eCellType::ABSOLUTE: sumAbsolute += (unsigned)std::max(0.0f, child.size.value); break;
			case eCellType::WEIGHT: sumPercentage += std::max(0.0f, child.size.value); break;
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
		switch (child.size.type) {
			case eCellType::ABSOLUTE: moving = (int)std::max(0.0f, child.size.value); break;
			case eCellType::WEIGHT: moving = (int)std::max(0.0f, child.size.value)/sumPercentage*weightedLength; break;
			case eCellType::AUTO: moving = 80; break;
		}

		int totalMarginMoving = child.size.margin.left + child.size.margin.right;
		int totalMarginFix = child.size.margin.top + child.size.margin.bottom;

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
			int fixLowMargin = !m_vertical ? child.size.margin.bottom : child.size.margin.left;
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

void LinearLayout::OnAttach(Control* parent) {
	Layout::OnAttach(parent);
	for (auto& child : m_children) {
		Attach(this, child.control.get());
	}
}

void LinearLayout::OnDetach() {
	for (auto& child : m_children) {
		Detach(child.control.get());
	}
	Layout::OnDetach();
}


} // namespace inl::gui
