#include "ScrollFrameV.hpp"

#include "Placeholders/PlaceholderOverlayEntity.hpp"


namespace inl::gui {


ScrollFrameV::ScrollFrameV() {
	AddChild(m_scrollLayout);

	m_scrollLayout.SetDirection(LinearLayout::HORIZONTAL);
	m_scrollLayout.AddChild(m_contentLayout);
	m_scrollLayout.AddChild(m_scrollBar);
	m_scrollLayout[&m_contentLayout].SetWeight(1.0f).SetMargin({ 0, 0, 0, 0 }).MoveToBack();
	m_scrollLayout[&m_scrollBar].SetWidth(m_scrollBarWidth).SetMargin({ 0, 0, 0, 0 }).MoveToBack();

	m_scrollBar.SetDirection(ScrollBar::VERTICAL);
	m_scrollBar.SetInverted(true);

	m_contentLayout.SetReferencePoint(AbsoluteLayout::eRefPoint::TOPLEFT);
	m_contentLayout.SetYDown(true);

	m_scrollBar.OnChanged += [this](float begin) {
		UpdateContentPosition();
	};

	OnMouseWheel += [this](Control*, Vec2, float amount) {
		m_scrollBar.SetVisiblePosition(m_scrollBar.GetVisiblePosition() + -60.f * amount);
	};
}


void ScrollFrameV::SetSize(const Vec2& size) {
	m_scrollLayout.SetSize(size);
	m_scrollBar.SetVisibleLength(size.y);
	UpdateContentPosition();
}


Vec2 ScrollFrameV::GetSize() const {
	return m_scrollLayout.GetSize();
}


Vec2 ScrollFrameV::GetMinimumSize() const {
	return m_scrollLayout.GetMinimumSize();
}


Vec2 ScrollFrameV::GetPreferredSize() const {
	return m_scrollLayout.GetPreferredSize();
}


void ScrollFrameV::SetPosition(const Vec2& position) {
	m_scrollLayout.SetPosition(position);
}


Vec2 ScrollFrameV::GetPosition() const {
	return m_scrollLayout.GetPosition();
}


void ScrollFrameV::Update(float elapsed) {
}


void ScrollFrameV::SetContent(std::shared_ptr<Control> content) {
	m_content = content;
	m_contentLayout.ClearChildren();
	if (content) {
		m_contentLayout.AddChild(m_content);
	}
}


std::shared_ptr<Control> ScrollFrameV::GetContent() const {
	return m_content;
}


void ScrollFrameV::SetContentHeight(float height) {
	m_contentHeight = height;
	m_scrollBar.SetTotalLength(height);
	UpdateContentPosition();
}


float ScrollFrameV::SetDepth(float depth) {
	return m_scrollLayout.SetDepth(depth);
}


float ScrollFrameV::GetDepth() const {
	return m_scrollLayout.GetDepth();
}


void ScrollFrameV::UpdateContentPosition() {
	if (!m_content) {
		return;
	}

	// Set width.
	float contentWidth = m_scrollLayout.GetSize().x - m_scrollBarWidth;
	m_content->SetSize({ contentWidth, m_contentHeight });


	auto& binding = m_contentLayout[m_content.get()];
	binding.SetPosition({ contentWidth / 2.0f, m_contentHeight / 2.0f - m_scrollBar.GetVisiblePosition() });
}


} // namespace inl::gui