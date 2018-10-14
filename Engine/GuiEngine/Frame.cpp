#include "Frame.hpp"
#include "Placeholders/PlaceholderOverlayEntity.hpp"


namespace inl::gui {


Frame::Frame() {
	m_background.reset(new PlaceholderOverlayEntity());
	m_background->SetColor({ 0.1f, 0.1f, 0.1f, 1.0f });
	m_background->SetZDepth(-0.1f);
}


void Frame::SetSize(Vec2u size) {
	m_background->SetScale(size);
	if (m_layout) {
		m_layout->SetSize(size);
	}
}


Vec2u Frame::GetSize() const {
	return m_background->GetScale();
}


void Frame::SetPosition(Vec2i position) {
	m_background->SetPosition(position);
	if (m_layout) {
		m_layout->SetPosition(position);
	}
}


Vec2i Frame::GetPosition() const {
	return m_background->GetPosition();
}


void Frame::Update(float elapsed) {
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

void Frame::SetZOrder(int rank) {
	m_background->SetZDepth(rank);
}

std::vector<std::reference_wrapper<std::unique_ptr<gxeng::ITextEntity>>> Frame::GetTextEntities() {
	return {};
}

std::vector<std::reference_wrapper<std::unique_ptr<gxeng::IOverlayEntity>>> Frame::GetOverlayEntities() {
	return { m_background };
}


} // namespace inl::gui