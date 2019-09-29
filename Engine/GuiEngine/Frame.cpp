#include "Frame.hpp"

#include "Placeholders/PlaceholderOverlayEntity.hpp"


namespace inl::gui {


Frame::Frame() {
	AddChild(m_background);
}


void Frame::SetSize(const Vec2& size) {
	m_background.SetSize(size);
	if (m_layout) {
		m_layout->SetSize(size);
	}
}


Vec2 Frame::GetSize() const {
	return m_background.GetSize();
}

Vec2 Frame::GetMinimumSize() const {
	return m_layout ? m_layout->GetMinimumSize() : Vec2{ 0.f, 0.f };
}

Vec2 Frame::GetPreferredSize() const {
	return m_layout ? m_layout->GetPreferredSize() : Vec2{ 0.f, 0.f };
}


void Frame::SetPosition(const Vec2& position) {
	m_background.SetPosition(position);
	if (m_layout) {
		m_layout->SetPosition(position);
	}
}


Vec2 Frame::GetPosition() const {
	return m_background.GetPosition();
}


void Frame::SetLayout(std::shared_ptr<Layout> layout) {
	if (m_layout) {
		RemoveChild(m_layout.get());
		m_layout.reset();
	}

	AddChild(layout);
	m_layout = layout;
	if (m_layout) {
		m_layout->SetSize(GetSize());
		m_layout->SetPosition(GetPosition());
		m_layout->SetVisible(GetVisible());
	}
}


std::shared_ptr<Layout> Frame::GetLayout() const {
	return m_layout;
}


void Frame::UpdateStyle() {
	m_background.SetColor(GetStyle().background.v);
}


void Frame::ShowBackground(bool show) {
	m_background.SetVisible(show);
}


bool Frame::IsShowingBackground() {
	return m_background.GetVisible();
}


float Frame::SetDepth(float depth) {
	m_background.SetDepth(depth);
	float span = 1.0f;
	if (m_layout) {
		span += m_layout->SetDepth(depth + 1.0f);
	}
	return span;
}

float Frame::GetDepth() const {
	return m_background.GetDepth();
}


} // namespace inl::gui