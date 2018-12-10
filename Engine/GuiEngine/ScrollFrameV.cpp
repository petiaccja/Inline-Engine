#include "ScrollFrameV.hpp"

#include "Placeholders/PlaceholderOverlayEntity.hpp"


namespace inl::gui {


ScrollFrameV::ScrollFrameV() {
	m_scrollLayout.SetDirection(LinearLayout::HORIZONTAL);
	m_scrollLayout.PushBack(m_contentLayout, LinearLayout::CellSize().SetWeight(1.0f).SetMargin({ 0, 0, 0, 0 }));
	m_scrollLayout.PushBack(m_scrollBar, LinearLayout::CellSize().SetWidth(m_scrollBarWidth).SetMargin({ 0, 0, 0, 0 }));

	m_scrollBar.SetDirection(ScrollBar::VERTICAL);
	m_scrollBar.SetInverted(true);

	m_contentLayout.SetReferencePoint(AbsoluteLayout::eRefPoint::TOPLEFT);
	m_contentLayout.SetYDown(true);

	m_scrollBar.OnChanged += [this](float begin) {
		UpdateContentPosition();
	};
}


void ScrollFrameV::SetSize(Vec2 size) {
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


void ScrollFrameV::SetPosition(Vec2 position) {
	m_scrollLayout.SetPosition(position);
}


Vec2 ScrollFrameV::GetPosition() const {
	return m_scrollLayout.GetPosition();
}


void ScrollFrameV::Update(float elapsed) {
	UpdateClip();
}


std::vector<const Control*> ScrollFrameV::GetChildren() const {
	return { &m_scrollLayout };
}


void ScrollFrameV::SetContent(std::shared_ptr<Control> content) {
	m_content = content;
	m_contentLayout.Clear();
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
}


void ScrollFrameV::OnAttach(Control* parent) {
	StandardControl::OnAttach(parent);
	Control::Attach(this, &m_scrollLayout);	
}


void ScrollFrameV::OnDetach() {
	Control::Detach(&m_scrollLayout);
	StandardControl::OnDetach();
}


float ScrollFrameV::SetDepth(float depth) {
	return m_scrollLayout.SetDepth(depth);
}


float ScrollFrameV::GetDepth() const {
	return m_scrollLayout.GetDepth();
}


std::vector<std::reference_wrapper<std::unique_ptr<gxeng::ITextEntity>>> ScrollFrameV::GetTextEntities() {
	return {};
}


std::vector<std::reference_wrapper<std::unique_ptr<gxeng::IOverlayEntity>>> ScrollFrameV::GetOverlayEntities() {
	return {};
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