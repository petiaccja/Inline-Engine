#include "Frame.hpp"
#include "Placeholders/PlaceholderOverlayEntity.hpp"


namespace inl::gui {


Frame::Frame() {
	m_background.reset(new PlaceholderOverlayEntity());
	m_background->SetColor({ 0.1f, 0.1f, 0.1f, 1.0f });
	m_background->SetZDepth(-0.1f);
}


void Frame::SetSize(Vec2 size) {
	m_background->SetScale(size);
	if (m_layout) {
		m_layout->SetSize(size);
	}
}


Vec2 Frame::GetSize() const {
	return m_background->GetScale();
}

Vec2 Frame::GetMinimumSize() const {
	return m_layout ? m_layout->GetMinimumSize() : Vec2{ 0.f, 0.f };
}

Vec2 Frame::GetPreferredSize() const {
	return m_layout ? m_layout->GetPreferredSize() : Vec2{ 0.f, 0.f };
}


void Frame::SetPosition(Vec2 position) {
	m_background->SetPosition(position);
	if (m_layout) {
		m_layout->SetPosition(position);
	}
}


Vec2 Frame::GetPosition() const {
	return m_background->GetPosition();
}


void Frame::Update(float elapsed) {
	UpdateClip();

	if (m_layout) {
		m_layout->Update(elapsed);
	}
	m_background->SetColor(GetStyle().background.v);
}

std::vector<const Control*> Frame::GetChildren() const {
	if (m_layout) {
		return { m_layout.get() };
	}
	else {
		return {};
	}
}


void Frame::SetLayout(std::shared_ptr<Layout> layout) {
	if (m_layout) {
		Detach(m_layout.get());
	}
	m_layout = layout;
	if (m_layout) {
		m_layout->SetSize(GetSize());
		m_layout->SetPosition(GetPosition());
		m_layout->SetVisible(GetVisible());
		Attach(this, m_layout.get());
	}
}

std::shared_ptr<Layout> Frame::GetLayout() const {
	return m_layout;
}

void Frame::OnAttach(Control* parent) {
	StandardControl::OnAttach(parent);
	if (m_layout) {
		Control::Attach(this, m_layout.get());
	}
}

void Frame::OnDetach() {
	if (m_layout) {
		Control::Detach(m_layout.get());
	}
	StandardControl::OnDetach();
}

float Frame::SetDepth(float depth) {
	m_background->SetZDepth(depth);
	float span = 1.0f;
	if (m_layout) {
		span += m_layout->SetDepth(depth + 1.0f);
	}
	return span;
}

float Frame::GetDepth() const {
	return m_background->GetZDepth();
}

std::vector<std::reference_wrapper<std::unique_ptr<gxeng::ITextEntity>>> Frame::GetTextEntities() {
	return {};
}

std::vector<std::reference_wrapper<std::unique_ptr<gxeng::IOverlayEntity>>> Frame::GetOverlayEntities() {
	return { m_background };
}


} // namespace inl::gui